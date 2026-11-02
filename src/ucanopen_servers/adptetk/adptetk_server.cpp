#include "adptetk_server.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <ucanopen/od/pdo_codec.h>
#include <vector>

namespace adptetk {

namespace {

using ucanopen::od::Description;
using ucanopen::od::Direction;
using ucanopen::od::Pdo;
using ucanopen::od::ValueTable;

// The RPDOs go thrice within the timeout of the device, 350 ms.
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

// Whether the table numbers the enumerators as the enumeration does, each
// named as its value; it says where it does not.
template<size_t N>
bool numbers_as(ValueTable const* table,
                std::array<std::string_view, N> const& names,
                std::string_view what,
                std::string const& server) {
  if (table == nullptr) {
    bsclog::error("{}: у сигнала {} нет таблицы значений.", server, what);
    return false;
  }
  bool same = true;
  for (size_t i = 0; i < N; ++i) {
    auto const* value = table->find(static_cast<int64_t>(i));
    if (value == nullptr || value->name != names[i]) {
      bsclog::error("{}: таблица значений {} называет {} иначе, чем «{}».",
                    server,
                    what,
                    i,
                    names[i]);
      same = false;
    }
  }
  return same;
}

} // namespace

Server::Server(std::shared_ptr<can::Socket> socket,
               ucanopen::NodeId node_id,
               std::string const& name,
               std::shared_ptr<ucanopen::od::Description const> description)
    : ucanopen::Server(socket,
                       node_id,
                       name,
                       object_dictionary,
                       std::move(description)) {
  auto const& od = *this->description();

  _drive_status_pdo = find_pdo(od, "drive_status", Direction::tx, name);
  _electrical_status_pdo =
      find_pdo(od, "electrical_status", Direction::tx, name);
  _motion_status_pdo = find_pdo(od, "motion_status", Direction::tx, name);
  _drive_command_pdo = find_pdo(od, "drive_command", Direction::rx, name);
  _model_command_pdo = find_pdo(od, "model_command", Direction::rx, name);

  has_signals(_drive_status_pdo,
              {"state",
               "pwm_on",
               "warning",
               "error",
               "critical",
               "emergency",
               "control_mode",
               "model_mode"},
              name);
  has_signals(_electrical_status_pdo, {"dc_voltage", "phase_current"}, name);
  has_signals(_motion_status_pdo, {"speed", "angle"}, name);

  _drive_states = table_of(od, _drive_status_pdo, "state");
  _control_modes = table_of(od, _drive_status_pdo, "control_mode");
  _model_modes = table_of(od, _drive_status_pdo, "model_mode");
  numbers_as(_drive_states, drive_state_names, "drive_status.state", name);

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
  _drive_status_tpdo =
      register_tpdo(_drive_status_pdo, &Server::_handle_drive_status);
  register_tpdo(_electrical_status_pdo, &Server::_handle_electrical_status);
  register_tpdo(_motion_status_pdo, &Server::_handle_motion_status);

  // An RPDO goes only if every signal of it is filled and its modes are
  // numbered as the panel numbers them: a frame the device would read
  // otherwise must not go at all.
  auto register_rpdo = [this, &name](Pdo const* pdo,
                                     std::set<std::string, std::less<>> const&
                                         names,
                                     bool modes_match,
                                     auto creator) {
    if (pdo == nullptr) {
      return;
    }
    auto const rpdo = ucanopen::od::rpdo_of(*pdo);
    auto const refusal = ucanopen::od::encoding_refusal(*pdo, names);
    if (!rpdo.has_value()) {
      bsclog::error("{}: PDO «{}» не из RPDO1–RPDO4 предопределённого набора, "
                    "он не шлётся.",
                    name,
                    pdo->name);
    } else if (refusal.has_value()) {
      bsclog::error("{}: RPDO «{}» не шлётся: {}.", name, pdo->name, *refusal);
    } else if (!modes_match) {
      bsclog::error("{}: RPDO «{}» не шлётся: режимы нумеруются иначе.",
                    name,
                    pdo->name);
    } else {
      rpdo_service.register_rpdo(*rpdo, rpdo_period, [this, creator]() {
        return (this->*creator)();
      });
    }
  };
  register_rpdo(
      _drive_command_pdo,
      {"emergency", "power", "start", "control_mode", "torque_ref", "speed_ref"},
      numbers_as(table_of(od, _drive_command_pdo, "control_mode"),
                 control_mode_names,
                 "drive_command.control_mode",
                 name),
      &Server::_create_drive_command);
  register_rpdo(_model_command_pdo,
                {"angle_ref", "current_ref", "voltage_ref", "model_mode"},
                numbers_as(table_of(od, _model_command_pdo, "model_mode"),
                           model_mode_names,
                           "model_command.model_mode",
                           name),
                &Server::_create_model_command);
}

std::string const& Server::model_mode_label(ModelMode mode) const {
  static std::string const unknown = "н/д";
  if (_model_modes == nullptr) {
    return unknown;
  }
  auto const* value = _model_modes->find(std::to_underlying(mode));
  if (value == nullptr) {
    return unknown;
  }
  return value->label.ru.empty() ? value->name : value->label.ru;
}

std::string_view Server::_label(ucanopen::od::ValueTable const* table,
                                int64_t value) {
  auto const* named = (table == nullptr) ? nullptr : table->find(value);
  if (named == nullptr) {
    return "н/д";
  }
  return named->label.ru.empty() ? named->name : named->label.ru;
}

//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_drive_status(ucanopen::can_payload const& payload) {
  auto const decoded =
      ucanopen::od::decode_pdo(*description(), *_drive_status_pdo, payload);
  if (!decoded.has_value()) {
    return;
  }
  for (auto const& value : decoded->signals) {
    auto const& name = value.signal->name;
    if (name == "state") {
      _drive_status.state.store(value.raw);
    } else if (name == "pwm_on") {
      _drive_status.pwm_on.store(value.raw != 0);
    } else if (name == "warning") {
      _drive_status.warning.store(value.raw != 0);
    } else if (name == "error") {
      _drive_status.error.store(value.raw != 0);
    } else if (name == "critical") {
      _drive_status.critical.store(value.raw != 0);
    } else if (name == "emergency") {
      _drive_status.emergency.store(value.raw != 0);
    } else if (name == "control_mode") {
      _drive_status.control_mode.store(value.raw);
    } else if (name == "model_mode") {
      _drive_status.model_mode.store(value.raw);
    }
  }
}

void Server::_handle_electrical_status(ucanopen::can_payload const& payload) {
  auto const decoded = ucanopen::od::decode_pdo(
      *description(), *_electrical_status_pdo, payload);
  if (!decoded.has_value()) {
    return;
  }
  for (auto const& value : decoded->signals) {
    auto const physical = static_cast<float>(
        ucanopen::od::physical(*value.signal, value.raw));
    if (value.signal->name == "dc_voltage") {
      _electrical_status.dc_voltage.store(physical);
    } else if (value.signal->name == "phase_current") {
      _electrical_status.phase_current.store(physical);
    }
  }
}

void Server::_handle_motion_status(ucanopen::can_payload const& payload) {
  auto const decoded =
      ucanopen::od::decode_pdo(*description(), *_motion_status_pdo, payload);
  if (!decoded.has_value()) {
    return;
  }
  for (auto const& value : decoded->signals) {
    auto const physical = static_cast<float>(
        ucanopen::od::physical(*value.signal, value.raw));
    if (value.signal->name == "speed") {
      _motion_status.speed.store(physical);
    } else if (value.signal->name == "angle") {
      _motion_status.angle.store(physical);
    }
  }
}

std::optional<ucanopen::can_payload> Server::_create_drive_command() {
  auto const control_mode = _drive_command.control_mode.load();
  return _build(
      *_drive_command_pdo,
      {{"emergency", _drive_command.emergency.load() ? 1.0 : 0.0},
       {"power", _drive_command.power.load() ? 1.0 : 0.0},
       {"start", _drive_command.start.load() ? 1.0 : 0.0},
       {"control_mode", static_cast<double>(std::to_underlying(control_mode))},
       {"torque_ref", _drive_command.torque_ref.load()},
       {"speed_ref", _drive_command.speed_ref.load()}},
      _drive_command.counter);
}

std::optional<ucanopen::can_payload> Server::_create_model_command() {
  auto const model_mode = _model_command.model_mode.load();
  return _build(
      *_model_command_pdo,
      {{"angle_ref", _model_command.angle_ref.load()},
       {"current_ref", _model_command.current_ref.load()},
       {"voltage_ref", _model_command.voltage_ref.load()},
       {"model_mode", static_cast<double>(std::to_underlying(model_mode))}},
      _model_command.counter);
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

} // namespace adptetk
