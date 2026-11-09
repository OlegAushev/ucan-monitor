#include "psfb_server.hpp"

#include <algorithm>
#include <chrono>
#include <ucanopen/od/pdo_codec.h>
#include <vector>

namespace psfb {

namespace {

using ucanopen::od::Description;
using ucanopen::od::Direction;
using ucanopen::od::Pdo;
using ucanopen::od::ValueTable;

// The RPDO goes thrice within the timeout of the device, 350 ms.
constexpr std::chrono::milliseconds rpdo_period{100};

// A TPDO counts as lost after five of its periods, and never sooner than in
// 500 ms.
std::chrono::milliseconds timeout_of(Pdo const& pdo) {
  return std::max(std::chrono::milliseconds(5 * pdo.period_ms),
                  std::chrono::milliseconds(500));
}

Pdo const* find_pdo(Description const& description,
                    std::string_view name,
                    Direction direction,
                    std::string const& server) {
  auto const* pdo = description.pdo(name);
  if (pdo == nullptr || pdo->direction != direction) {
    bsclog::error("{}: описание словаря не называет {} «{}».",
                  server,
                  (direction == Direction::tx) ? "TPDO" : "RPDO",
                  name);
    return nullptr;
  }
  return pdo;
}

ucanopen::od::Signal const* signal_of(Pdo const* pdo, std::string_view name) {
  if (pdo == nullptr) {
    return nullptr;
  }
  auto const signal =
      std::find_if(pdo->signals.begin(),
                   pdo->signals.end(),
                   [name](auto const& s) { return s.name == name; });
  return (signal == pdo->signals.end()) ? nullptr : &*signal;
}

// Whether `pdo` has every signal of `names`; it says which it lacks.
bool has_signals(Pdo const* pdo,
                 std::vector<std::string_view> const& names,
                 std::string const& server) {
  if (pdo == nullptr) {
    return false;
  }
  bool all = true;
  for (auto name : names) {
    if (signal_of(pdo, name) == nullptr) {
      bsclog::error("{}: у PDO «{}» нет сигнала «{}».", server, pdo->name, name);
      all = false;
    }
  }
  return all;
}

ValueTable const* table_of(Description const& description,
                           Pdo const* pdo,
                           std::string_view signal) {
  auto const* found = signal_of(pdo, signal);
  return (found == nullptr) ? nullptr
                            : description.value_table(found->value_table);
}

} // namespace

Server::Server(std::shared_ptr<can::Socket> socket,
               ucanopen::NodeId node_id,
               std::string const& name,
               std::shared_ptr<ucanopen::od::Description const> description)
    : ucanopen::Server(socket, node_id, name, std::move(description)) {
  auto const& od = *this->description();

  _converter_status_pdo = find_pdo(od, "converter_status", Direction::tx, name);
  _dc_voltages_pdo = find_pdo(od, "dc_voltages", Direction::tx, name);
  _output_currents_pdo = find_pdo(od, "output_currents", Direction::tx, name);
  _converter_command_pdo =
      find_pdo(od, "converter_command", Direction::rx, name);

  has_signals(_converter_status_pdo,
              {"state",
               "pwm1_on",
               "pwm2_on",
               "warning",
               "error",
               "critical",
               "emergency",
               "overlap1",
               "overlap2"},
              name);
  has_signals(_dc_voltages_pdo, {"hv_voltage", "lv_voltage"}, name);
  has_signals(_output_currents_pdo, {"current1", "current2"}, name);

  _converter_states = table_of(od, _converter_status_pdo, "state");

  auto register_tpdo = [this, &name](Pdo const* pdo, auto handler) {
    if (pdo == nullptr) {
      return std::optional<ucanopen::CobTpdo>{};
    }
    auto const tpdo = ucanopen::od::tpdo_of(*pdo);
    if (!tpdo.has_value()) {
      bsclog::error("{}: PDO «{}» не из TPDO1–TPDO4 предопределённого набора.",
                    name,
                    pdo->name);
      return tpdo;
    }
    tpdo_service.register_tpdo(
        *tpdo,
        timeout_of(*pdo),
        [this, handler](ucanopen::can_payload const& payload) {
          (this->*handler)(payload);
        });
    return tpdo;
  };
  _converter_status_tpdo =
      register_tpdo(_converter_status_pdo, &Server::_handle_converter_status);
  register_tpdo(_dc_voltages_pdo, &Server::_handle_dc_voltages);
  register_tpdo(_output_currents_pdo, &Server::_handle_output_currents);

  // The RPDO goes only if every signal of it is filled: a frame the device
  // would read otherwise must not go at all.
  if (_converter_command_pdo != nullptr) {
    auto const rpdo = ucanopen::od::rpdo_of(*_converter_command_pdo);
    auto const refusal = ucanopen::od::encoding_refusal(
        *_converter_command_pdo,
        {"emergency", "power", "start", "overlap1", "overlap2"});
    if (!rpdo.has_value()) {
      bsclog::error("{}: PDO «{}» не из RPDO1–RPDO4 предопределённого набора, "
                    "он не шлётся.",
                    name,
                    _converter_command_pdo->name);
    } else if (refusal.has_value()) {
      bsclog::error("{}: RPDO «{}» не шлётся: {}.",
                    name,
                    _converter_command_pdo->name,
                    *refusal);
    } else {
      rpdo_service.register_rpdo(*rpdo, rpdo_period, [this]() {
        return _create_converter_command();
      });
    }
  }
}

std::string_view Server::converter_state_str() const {
  auto const* named =
      (_converter_states == nullptr)
          ? nullptr
          : _converter_states->find(_converter_status.state.load());
  if (named == nullptr) {
    return "н/д";
  }
  return named->label.ru.empty() ? named->name : named->label.ru;
}

//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_converter_status(ucanopen::can_payload const& payload) {
  auto const decoded =
      ucanopen::od::decode_pdo(*description(), *_converter_status_pdo, payload);
  if (!decoded.has_value()) {
    return;
  }
  for (auto const& value : decoded->signals) {
    auto const& name = value.signal->name;
    if (name == "state") {
      _converter_status.state.store(value.raw);
    } else if (name == "pwm1_on") {
      _converter_status.pwm1_on.store(value.raw != 0);
    } else if (name == "pwm2_on") {
      _converter_status.pwm2_on.store(value.raw != 0);
    } else if (name == "warning") {
      _converter_status.warning.store(value.raw != 0);
    } else if (name == "error") {
      _converter_status.error.store(value.raw != 0);
    } else if (name == "critical") {
      _converter_status.critical.store(value.raw != 0);
    } else if (name == "emergency") {
      _converter_status.emergency.store(value.raw != 0);
    } else if (name == "overlap1") {
      _converter_status.overlap1.store(static_cast<float>(
          ucanopen::od::physical(*value.signal, value.raw)));
    } else if (name == "overlap2") {
      _converter_status.overlap2.store(static_cast<float>(
          ucanopen::od::physical(*value.signal, value.raw)));
    }
  }
}

void Server::_handle_dc_voltages(ucanopen::can_payload const& payload) {
  auto const decoded =
      ucanopen::od::decode_pdo(*description(), *_dc_voltages_pdo, payload);
  if (!decoded.has_value()) {
    return;
  }
  for (auto const& value : decoded->signals) {
    auto const physical = static_cast<float>(
        ucanopen::od::physical(*value.signal, value.raw));
    if (value.signal->name == "hv_voltage") {
      _dc_voltages.hv_voltage.store(physical);
    } else if (value.signal->name == "lv_voltage") {
      _dc_voltages.lv_voltage.store(physical);
    }
  }
}

void Server::_handle_output_currents(ucanopen::can_payload const& payload) {
  auto const decoded =
      ucanopen::od::decode_pdo(*description(), *_output_currents_pdo, payload);
  if (!decoded.has_value()) {
    return;
  }
  for (auto const& value : decoded->signals) {
    auto const physical = static_cast<float>(
        ucanopen::od::physical(*value.signal, value.raw));
    if (value.signal->name == "current1") {
      _output_currents.current1.store(physical);
    } else if (value.signal->name == "current2") {
      _output_currents.current2.store(physical);
    }
  }
}

std::optional<ucanopen::can_payload> Server::_create_converter_command() {
  return _build(*_converter_command_pdo,
                {{"emergency", _converter_command.emergency.load() ? 1.0 : 0.0},
                 {"power", _converter_command.power.load() ? 1.0 : 0.0},
                 {"start", _converter_command.start.load() ? 1.0 : 0.0},
                 {"overlap1", _converter_command.overlap1_ref.load()},
                 {"overlap2", _converter_command.overlap2_ref.load()}},
                _converter_command.counter);
}

std::optional<ucanopen::can_payload>
Server::_build(ucanopen::od::Pdo const& pdo,
               std::map<std::string, double, std::less<>> const& values,
               unsigned& counter) {
  auto const encoded =
      ucanopen::od::encode_pdo(*description(), pdo, values, counter);
  if (!encoded.has_value()) {
    if (_unbuilt.insert(pdo.name).second) {
      bsclog::error("{}: кадр RPDO «{}» не собран и не послан: {}.",
                    name(),
                    pdo.name,
                    encoded.error());
    }
    return std::nullopt;
  }
  _unbuilt.erase(pdo.name);

  for (auto const& [signal, value] : values) {
    auto const path = pdo.name + "." + signal;
    bool const saturated = std::find(encoded->saturated.begin(),
                                     encoded->saturated.end(),
                                     path) != encoded->saturated.end();
    if (!saturated) {
      _saturated.erase(path);
    } else if (_saturated.insert(path).second) {
      bsclog::warning("{}: задание {} = {} не умещается в сигнал и ограничено.",
                      name(),
                      path,
                      value);
    }
  }

  ++counter;
  return encoded->payload;
}

} // namespace psfb
