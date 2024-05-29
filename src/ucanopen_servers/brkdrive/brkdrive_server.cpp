#include "brkdrive_server.h"
#include <boost/crc.hpp>


namespace brkdrive {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service) {
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo1, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo2, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    // tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3, std::chrono::milliseconds(1000),
    //                            [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1, std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo1(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2, std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo2(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3, std::chrono::milliseconds(100),
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


//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(const ucanopen::can_payload& payload){
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

    _tpdo1.run.store(static_cast<bool>(tpdo.run));
    _tpdo1.error.store(static_cast<bool>(tpdo.error));
    _tpdo1.warning.store(static_cast<bool>(tpdo.warning));


    if (opmode_map.contains(tpdo.opmode)) {
        _tpdo1.opmode.store(opmode_map.at(tpdo.opmode));
    } else {
        _tpdo1.opmode.store("unknown");
    }

    if (ctlmode_map.contains(tpdo.ctlmode)) {
        _tpdo1.ctlmode.store(ctlmode_map.at(tpdo.ctlmode));
    } else {
        _tpdo1.ctlmode.store("unknown");
    }

    if (ctlloop_map.contains(tpdo.ctlloop)) {
        _tpdo1.ctlloop.store(ctlloop_map.at(tpdo.ctlloop));
    } else {
        _tpdo1.ctlloop.store("unknown");
    }

    if (tpdo.drive_state < drive_states.size()) {
        _tpdo1.drive_state = drive_states[tpdo.drive_state];
    } else {
        _tpdo1.drive_state.store("unknown");
    }

    _tpdo1.torque.store(static_cast<float>(tpdo.torque) / 10000.0f);
    _tpdo1.speed.store(tpdo.speed);
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo2) == 8);
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

    _tpdo2.braking.store(tpdo.braking / 10000.f);
}


void Server::_handle_tpdo4(const ucanopen::can_payload& payload){
    static_assert(sizeof(CobTpdo4) == 8);
    CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

    _tpdo4.errors.store(tpdo.errors);
    _tpdo4.warnings.store(tpdo.warnings);
}


//----------------------------------------------------------------------------------------------------------------------
ucanopen::can_payload Server::_create_rpdo1() {
    static_assert(sizeof(CobRpdo1) == 8);
    static unsigned int counter = 0;
    
    CobRpdo1 rpdo{};

    rpdo.wakeup = _rpdo1.wakeup.load();
    rpdo.brake_ref = 10000.0f * _rpdo1.brake_ref.load();

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo1>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo2() {
    static_assert(sizeof(CobRpdo2) == 8);
    static unsigned int counter = 0;

    CobRpdo2 rpdo{};

    rpdo.torque_ref = 10000.0f * _rpdo2.torque_ref.load();
    rpdo.speed_ref = _rpdo2.speed_ref.load();
    rpdo.dcurr_ref = 10000.0f * _rpdo2.dcurr_ref.load();

    rpdo.opmode = std::to_underlying(_rpdo2.opmode.load());
    rpdo.ctlmode = std::to_underlying(_rpdo2.ctlmode.load());
    rpdo.ctlloop = std::to_underlying(_rpdo2.ctlloop.load());
    rpdo.run = _rpdo2.run.load();

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo3() {
    static_assert(sizeof(CobRpdo3) == 8);
    static unsigned int counter = 0;

    CobRpdo3 rpdo{};

    rpdo.openloop_angle_ref = _rpdo3.openloop_angle_ref.load();
    rpdo.angle_ref = _rpdo3.angle_ref.load();
    rpdo.track_speed = _rpdo3.track_speed.load();

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo3>(rpdo);
}


} // namespace brkdrive
