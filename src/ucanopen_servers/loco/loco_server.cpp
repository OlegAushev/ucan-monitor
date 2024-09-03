#include "loco_server.h"
#include <utility>


namespace loco {


Server::Server(std::shared_ptr<can::Socket> socket,
               ucanopen::NodeId node_id,
               const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service) {
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo1,
                               std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo2,
                               std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3,
                               std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4,
                               std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1,
                               std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo1(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2,
                               std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo2(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3,
                               std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo3(); });
}


ucanopen::FrameHandlingStatus Server::handle_sdo(ucanopen::ODEntryIter entry,
                                                 [[maybe_unused]] ucanopen::SdoType sdo_type,
                                                 ucanopen::ExpeditedSdoData data) {
    if (entry->second.name == "syslog_message") {
        auto message_id = data.u32();
        if ((message_id != 0) && (message_id < syslog_messages.size())) {
            bsclog::info("{}", syslog_messages[message_id]);
        }
    }

    return ucanopen::FrameHandlingStatus::success;
}


void Server::_handle_tpdo1(const ucanopen::can_payload& payload){
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

    if (drive_state_values.contains(tpdo.drive_state)) {
        _tpdo1.drive_state.store(static_cast<DriveState>(tpdo.drive_state));
    }

    _tpdo1.pwm_on.store(static_cast<bool>(tpdo.pwm_on));
    _tpdo1.error.store(static_cast<bool>(tpdo.error));
    _tpdo1.warning.store(static_cast<bool>(tpdo.warning));

    if (ctlmode_values.contains(tpdo.ctlmode)) {
        _tpdo1.ctlmode.store(static_cast<ControlMode>(tpdo.ctlmode));
    }

    if (ctlloop_values.contains(tpdo.ctlloop)) {
        _tpdo1.ctlloop.store(static_cast<ControlLoop>(tpdo.ctlloop));
    }

    _tpdo1.torque = static_cast<float>(tpdo.torque) / 10000.f;
    _tpdo1.speed = tpdo.speed;

    _tpdo1.manual_field = static_cast<bool>(tpdo.manual_field);
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo2) == 8);
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);
}


void Server::_handle_tpdo3(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo3) == 8);
    CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);
}

void Server::_handle_tpdo4(const ucanopen::can_payload &payload) {
    static_assert(sizeof(CobTpdo4) == 8);
    CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

    _tpdo4.errors.store(tpdo.errors);
    _tpdo4.warnings.store(tpdo.warnings);
}


ucanopen::can_payload Server::_create_rpdo1() {
    static_assert(sizeof(CobRpdo1) == 8);
    static unsigned int counter = 0;

    CobRpdo1 rpdo{};

    rpdo.power = _rpdo1.power.load();
    rpdo.start = _rpdo1.start.load();
    rpdo.ctlmode = std::to_underlying(_rpdo1.ctlmode.load());
    rpdo.ref_torque = _rpdo1.ref_torque.load() * 10000.f;
    rpdo.ref_speed = static_cast<float>(_rpdo1.ref_speed.load());

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo1>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo2() {
    static_assert(sizeof(CobRpdo2) == 8);
    static unsigned int counter = 0;

    CobRpdo2 rpdo{};

    rpdo.opmode = std::to_underlying(_rpdo2.opmode.load());
    rpdo.ctlloop = std::to_underlying(_rpdo2.ctlloop.load());
    rpdo.ref_d_angle = _rpdo2.ref_d_angle.load();
    rpdo.ref_d_current = _rpdo2.ref_d_current.load() * 10000.f;

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo3() {
    static_assert(sizeof(CobRpdo3) == 8);
    static unsigned int counter = 0;

    CobRpdo3 rpdo{};

    rpdo.manual_field = _rpdo3.manual_field.load();
    rpdo.ref_f_current = _rpdo3.ref_f_current.load() * 10000.f;

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo3>(rpdo);
}




} // namespace loco
