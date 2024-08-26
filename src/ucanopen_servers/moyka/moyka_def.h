#pragma once


#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace moyka {


struct CobTpdo1 {
    uint16_t statusRun : 1;	// bit 0
    uint16_t statusFault : 1;	// bit 1
    uint16_t statusWarning : 1;	// bit 2
    uint16_t statusOverheat : 1;	// bit 3
    uint16_t statusReserved : 4;	// bit 4..7
    int8_t torque : 8;
    int16_t speed : 16;
    int8_t currentS : 8;
    int8_t power : 8;
    uint8_t voltageDC : 8;
    uint8_t currentF : 8;
};


struct CobTpdo2 {
    uint8_t tempMotorS : 8;
    uint8_t reserved1 : 8;
    uint8_t voltageOut : 8;
    uint8_t reserved2 : 7;
    uint16_t torqueLimitation : 1;
    uint8_t tempMotorFw : 8;
    uint8_t tempHeatsink : 8;
    uint8_t tepmCaseAir : 8;
    uint8_t reserved3 : 8;
};


struct CobTpdo3 {
    uint8_t voltagePosHousing : 8;
    uint8_t voltageNegHousing : 8;
    uint16_t statusReserved1 : 1;			// bit 0
    uint16_t statusInsulationLow : 1;		// bit 1
    uint16_t statusInsulationLowWoFilter : 1;	// bit 2
    uint16_t statusReserved2 : 1;			// bit 3
    uint16_t reserved1 : 3;
    uint16_t driveReference : 1;			// by protocol: uint16_t reserved1 : 4;
    int8_t currentDC : 8;
    uint32_t syslogInfo : 32;			// by protocol: uint32_t reserved2 : 32;
};


struct CobTpdo4 {
    uint32_t faultCode : 32;
    uint16_t nWarnings : 16;
    uint16_t warningCode : 16;
};


enum class DriveState {
	STANDBY,
	IDLE,
	POWERUP,
	READY,
	PREPARING,
	STARTING,
	IN_OPERATION,
	STOPPING,
	POWERDOWN,
	CALIBRATING_POS_SENSOR_POWERUP,
	CALIBRATING_POS_SENSOR,
	EVALUATING_MOTOR,
};


inline const std::unordered_set<int> drive_state_values {
    std::to_underlying(DriveState::STANDBY),
    std::to_underlying(DriveState::IDLE),
    std::to_underlying(DriveState::POWERUP),
    std::to_underlying(DriveState::READY),
    std::to_underlying(DriveState::PREPARING),
    std::to_underlying(DriveState::STARTING),
    std::to_underlying(DriveState::IN_OPERATION),
    std::to_underlying(DriveState::STOPPING),
    std::to_underlying(DriveState::POWERDOWN),
    std::to_underlying(DriveState::CALIBRATING_POS_SENSOR_POWERUP), 
    std::to_underlying(DriveState::CALIBRATING_POS_SENSOR), 
    std::to_underlying(DriveState::EVALUATING_MOTOR) 
};


inline const std::unordered_map<DriveState, std::string_view> drive_state_names = {
    {DriveState::STANDBY, "ОЖИДАНИЕ"},
    {DriveState::IDLE, "ОЖИДАНИЕ"},
    {DriveState::POWERUP, "ВКЛЮЧЕНИЕ"},
    {DriveState::READY, "ГОТОВ"},
    {DriveState::PREPARING, "ЗАПУСК"},
    {DriveState::STARTING, "ЗАПУСК"},
    {DriveState::IN_OPERATION, "РАБОТА"},
    {DriveState::STOPPING, "ОСТАНОВ"},
    {DriveState::POWERDOWN, "ВЫКЛЮЧЕНИЕ"},
    {DriveState::CALIBRATING_POS_SENSOR_POWERUP, "КАЛИБРОВКА"},
    {DriveState::CALIBRATING_POS_SENSOR, "КАЛИБРОВКА"},
    {DriveState::EVALUATING_MOTOR, "КАЛИБРОВКА"},
};


} // namespace moyka
