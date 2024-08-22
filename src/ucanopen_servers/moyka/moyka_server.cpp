#include "moyka_server.h"
#include <boost/crc.hpp>


namespace moyka {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service) {
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo1, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });
}


ucanopen::FrameHandlingStatus Server::handle_sdo(ucanopen::ODEntryIter entry,
                                                 [[maybe_unused]] ucanopen::SdoType sdo_type,
                                                 ucanopen::ExpeditedSdoData data) {
    if (entry->second.name == "DRIVE_STATE") {
        if (drive_state_values.contains(data.u32()))
        _drive_state.store(static_cast<DriveState>(data.u32()));
    }
    return ucanopen::FrameHandlingStatus::success;
}


//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(const ucanopen::can_payload& payload){
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);
    
    _tpdo1.pwm_on.store(tpdo.statusRun);
    _tpdo1.error.store(tpdo.statusFault);
    _tpdo1.dc_voltage.store((static_cast<float>(tpdo.voltageDC) / 255.f) * 1620.f);
    _tpdo1.speed.store((static_cast<float>(tpdo.speed) / 32767.f) * 8000.f);
}


void Server::_handle_tpdo4(const ucanopen::can_payload& payload){
    static_assert(sizeof(CobTpdo4) == 8);
    CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

    _tpdo4.errors.store(tpdo.faultCode);
    _tpdo4.warnings.store(tpdo.warningCode);
}


} // namespace brkdrive
