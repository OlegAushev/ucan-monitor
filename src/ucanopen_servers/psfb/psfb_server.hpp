#pragma once

#include <atomic>
#include <bsclog/bsclog.h>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <ucanopen/od/description.h>
#include <ucanopen/server/server.h>

namespace psfb {

// The ADPT-ETK PSFB converter. Its PDOs are those its description states: the
// state of the converter, its bridges and its overlaps come in
// converter_status, the voltages of both sides in dc_voltages, the currents
// of both channels in output_currents, the statuses in the MPDO that
// ucanopen::Server takes, and the commands go in converter_command, set in
// physical units.
class Server : public ucanopen::Server {
public:
  Server(std::shared_ptr<can::Socket> socket,
         ucanopen::NodeId node_id,
         std::string const& name,
         std::shared_ptr<ucanopen::od::Description const> description);
private:
  ucanopen::od::Pdo const* _converter_status_pdo{nullptr};
  ucanopen::od::Pdo const* _dc_voltages_pdo{nullptr};
  ucanopen::od::Pdo const* _output_currents_pdo{nullptr};
  ucanopen::od::Pdo const* _converter_command_pdo{nullptr};
  std::optional<ucanopen::CobTpdo> _converter_status_tpdo;

  // The table that names the states.
  ucanopen::od::ValueTable const* _converter_states{nullptr};

  struct {
    std::atomic<bool> emergency{false};
    std::atomic<bool> power{false};
    std::atomic<bool> start{false};
    std::atomic<float> overlap1_ref{0}; // pu
    std::atomic<float> overlap2_ref{0}; // pu
    unsigned counter{0};
  } _converter_command;

  struct {
    std::atomic<int64_t> state{-1}; // as its table numbers it, -1 before a frame
    std::atomic<bool> pwm1_on{false};
    std::atomic<bool> pwm2_on{false};
    std::atomic<bool> emergency{false};
    std::atomic<bool> critical{false};
    std::atomic<bool> error{false};
    std::atomic<bool> warning{false};
    std::atomic<float> overlap1{0}; // pu
    std::atomic<float> overlap2{0}; // pu
  } _converter_status;

  struct {
    std::atomic<float> hv_voltage{0}; // V
    std::atomic<float> lv_voltage{0}; // V
  } _dc_voltages;

  struct {
    std::atomic<float> current1{0}; // A
    std::atomic<float> current2{0}; // A
  } _output_currents;

  // What the RPDO reports once, while it lasts; the CAN thread alone builds
  // its frames.
  std::set<std::string> _unbuilt;
  std::set<std::string> _saturated;
public:
  // RPDO
  void toggle_emergency(bool v) { _converter_command.emergency.store(v); }

  void toggle_power(bool v) { _converter_command.power.store(v); }

  void toggle_start(bool v) { _converter_command.start.store(v); }

  void set_overlap1_ref(float pu) { _converter_command.overlap1_ref.store(pu); }

  void set_overlap2_ref(float pu) { _converter_command.overlap2_ref.store(pu); }

  // TPDO
  bool converter_status_good() const {
    return _converter_status_tpdo.has_value() &&
           tpdo_service.good(*_converter_status_tpdo);
  }

  std::string_view converter_state_str() const;

  bool is_pwm1_on() const { return _converter_status.pwm1_on.load(); }

  bool is_pwm2_on() const { return _converter_status.pwm2_on.load(); }

  bool has_error() const { return _converter_status.error.load(); }

  bool has_any_warning() const { return _converter_status.warning.load(); }

  bool has_emergency() const { return _converter_status.emergency.load(); }

  bool has_critical() const { return _converter_status.critical.load(); }

  float overlap1() const { return _converter_status.overlap1.load(); }

  float overlap2() const { return _converter_status.overlap2.load(); }

  float hv_voltage() const { return _dc_voltages.hv_voltage.load(); }

  float lv_voltage() const { return _dc_voltages.lv_voltage.load(); }

  float current1() const { return _output_currents.current1.load(); }

  float current2() const { return _output_currents.current2.load(); }
private:
  void _handle_converter_status(ucanopen::can_payload const& payload);
  void _handle_dc_voltages(ucanopen::can_payload const& payload);
  void _handle_output_currents(ucanopen::can_payload const& payload);

  std::optional<ucanopen::can_payload> _create_converter_command();
  std::optional<ucanopen::can_payload>
  _build(ucanopen::od::Pdo const& pdo,
         std::map<std::string, double, std::less<>> const& values,
         unsigned& counter);
};

} // namespace psfb
