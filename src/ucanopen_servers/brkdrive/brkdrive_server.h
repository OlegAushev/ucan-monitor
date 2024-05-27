#pragma once


#include "brkdrive_def.h"
#include "ucanopen/server/server.h"
#include "bsclog/bsclog.h"
#include <algorithm>
#include <atomic>


namespace brkdrive {

extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;


class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);
protected:
    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override final;
private:
    struct {
        std::atomic<bool> wakeup{false};
        std::atomic<float> brake_ref{0.0f};
    } _rpdo1;

    struct {
        std::atomic<float> torque_ref{0.f};
        std::atomic<int16_t> speed_ref{0};
        std::atomic<float> dcurr_ref{0.f};
        std::atomic<OperationMode> opmode{OperationMode::normal};
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
        std::atomic<bool> run{false};
    } _rpdo2;

    struct {
        std::atomic<int16_t> openloop_angle_ref{0};
        std::atomic<int16_t> angle_ref{0};
        std::atomic<uint16_t> track_speed{0};
    } _rpdo3;

    struct {
        std::atomic<std::optional<bool>> run{std::nullopt};
        std::atomic<std::optional<bool>> error{std::nullopt};
        std::atomic<std::optional<bool>> warning{std::nullopt};
        std::atomic<std::string_view> opmode{"n/a"};
        std::atomic<std::string_view> ctlmode{"n/a"};
        std::atomic<std::string_view> ctlloop{"n/a"};
        std::atomic<std::string_view> drive_state{"n/a"};
        std::atomic<std::optional<float>> torque{std::nullopt};
        std::atomic<std::optional<int16_t>> speed{std::nullopt};
    } _tpdo1;

    struct {
        std::atomic<std::optional<float>> braking{std::nullopt};
    } _tpdo2;

    struct {
        std::atomic<std::optional<uint32_t>> errors{std::nullopt};
        std::atomic<std::optional<uint16_t>> warnings{std::nullopt};
    } _tpdo4;

public:
    void toggle_wakeup(bool value) { _rpdo1.wakeup.store(value); }
    void set_brake_ref(float value) { _rpdo1.brake_ref.store(std::clamp(value, 0.0f, 1.0f)); }

    void set_torque_ref(float value) { _rpdo2.torque_ref.store(std::clamp(value, -1.0f, 1.0f)); }
    void set_speed_ref(int16_t value) { _rpdo2.speed_ref.store(value); }
    void set_dcurr_ref(float value) { _rpdo2.dcurr_ref.store(std::clamp(value, -1.0f, 1.0f)); }
    void set_opmode(OperationMode mode) { _rpdo2.opmode.store(mode); }
    void set_ctlmode(ControlMode mode) { _rpdo2.ctlmode.store(mode); }
    void set_ctlloop(ControlLoop loop) { _rpdo2.ctlloop.store(loop); }
    void toggle_run(bool value) { _rpdo2.run.store(value); }

    void set_openloop_angle_ref(int16_t value) { _rpdo3.openloop_angle_ref.store(value); }
    void set_angle_ref(int16_t value) { _rpdo3.angle_ref.store(value); }
    void set_track_speed(uint16_t value) { _rpdo3.track_speed.store(value); }

    std::optional<bool> is_running() const { return _tpdo1.run.load(); }
    std::optional<bool> has_error() const { return _tpdo1.error.load(); }
    std::optional<bool> has_warning() const { return _tpdo1.warning.load(); }
    std::string_view opmode() const { return _tpdo1.opmode.load(); }
    std::string_view ctlmode() const { return _tpdo1.ctlmode.load(); }
    std::string_view ctlloop() const { return _tpdo1.ctlloop.load(); }
    std::string_view drive_state() const { return _tpdo1.drive_state.load(); }
    std::optional<float> torque() const { return _tpdo1.torque.load(); }
    std::optional<int16_t> speed() const { return _tpdo1.speed.load(); }

    std::optional<float> braking_force() const { return _tpdo2.braking.load(); }

    std::optional<uint32_t> errors() const { return _tpdo4.errors.load(); }
    std::optional<uint16_t> warnings() const { return _tpdo4.warnings.load(); }

    const auto& error_list() const { return ::brkdrive::error_list; }
    const auto& warning_list() const { return ::brkdrive::warning_list; }

private:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    // void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();
    ucanopen::can_payload _create_rpdo3();
};

} // namespace brkdrive
