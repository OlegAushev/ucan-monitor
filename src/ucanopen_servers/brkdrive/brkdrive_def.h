#pragma once


#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


namespace brkdrive {


struct CobTpdo1 {
    float angle;

    uint8_t opstatus : 2;
    uint8_t calibrated : 1;
    uint8_t drive_state : 5;

    uint8_t pwm_on : 1;
    uint8_t error : 1;
    uint8_t warning : 1;
    uint8_t opmode : 2;
    uint8_t ctlmode : 1;
    uint8_t ctlloop : 2;
    
    uint8_t calstatus : 3;
    uint8_t trkcompleted : 1;
    uint8_t _reserved1 : 4;
    
    uint8_t counter : 2;
    uint8_t _reserved2 : 6;
};


struct CobTpdo2 {
    float ref_angle;
    
    uint8_t ref_brake;

    uint8_t _reserved1;

    uint8_t _reserved2;

    uint8_t counter : 2;
    uint8_t _reserved5 : 6;
};


struct CobTpdo3 {
    int16_t speed;

    uint8_t _reserved1;

    uint8_t _reserved2;

    uint8_t _reserved3;

    uint8_t _reserved4;

    uint8_t _reserved5;

    uint8_t counter : 2;
    uint8_t _reserved6 : 6;
};


struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint8_t _reserved1;
    uint8_t counter : 2;
    uint8_t _reserved2 : 6;
};


//----------------------------------------------------------------------------------------------------------------------
struct CobRpdo1 {
    float ref_angle;

    uint16_t track_speed;

    uint8_t master_bad : 1;
    uint8_t wakeup : 1;
    uint8_t cmd_opstatus : 2;
    uint8_t _reserved1 : 4;
    
    uint8_t counter : 2;
    uint8_t _reserved2 : 6;
};


struct CobRpdo2 {
    uint8_t opmode : 2;
    uint8_t ctlmode : 1;
    uint8_t ctlloop : 2;
    uint8_t _reserved1 : 3;
    
    uint8_t _reserved2;

    int16_t ref_torque;
    
    int16_t ref_speed;
    
    uint8_t _reserved3;
    
    uint8_t counter : 2;
    uint8_t _reserved4 : 6;
};


struct CobRpdo3 {
    int16_t ref_dcurr;

    int16_t openloop_ref_angle;
    
    uint8_t _reserved1;
    
    uint8_t _reserved2;
    
    uint8_t _reserved3;
    
    uint8_t counter : 2;
    uint8_t _reserved4 : 6;
};


//----------------------------------------------------------------------------------------------------------------------
inline const std::vector<std::string> syslog_messages = {
    "",
    "@syslog: [ info ] Boot CPU1...",
    "@syslog: [  ok  ] CPU1 is ready.",
    "@syslog: [ info ] Boot CPU2...",
    "@syslog: [  ok  ] CPU2 is ready.",
    "@syslog: [  ok  ] CPU1 periphery is configured.",
    "@syslog: [  ok  ] CPU2 periphery is configured.",
    "@syslog: [  ok  ] Device is ready.",
    "@syslog: [ fail ] Device is busy.",
    "@syslog: [ fail ] Error occurred.",
    "@syslog: [ info ] Device is resetting...",
    "@syslog: [ warn ] SDO request is lost.",
    "@syslog: [ warn ] Cannot communicate with external memory: disabled.",
    "@syslog: [  ok  ] Load settings - ok.",
    "@syslog: [ fail ] Load settings - fail.",
    "@syslog: [  ok  ] Apply settings - ok.",
    "@syslog: [ fail ] Apply settings - fail.",
    "@syslog: [  ok  ] Restore settings - ok.",
    "@syslog: [ fail ] Restore settings - fail.",
    "@syslog: [  ok  ] Load default settings - ok.",

    "@syslog: [  ok  ] Load ucanopen server config - ok.",
    "@syslog: [ fail ] Load ucanopen server config - fail.",
    "@syslog: [  ok  ] Load syslog config - ok.",
    "@syslog: [ fail ] Load syslog config - fail.",
    "@syslog: [  ok  ] Load drive config - ok.",
    "@syslog: [ fail ] Load drive config - fail.",
    "@syslog: [  ok  ] Load motor config - ok.",
    "@syslog: [ fail ] Load motor config - fail.",
    "@syslog: [  ok  ] Load converter config - ok.",
    "@syslog: [ fail ] Load converter config - fail.",
    "@syslog: [  ok  ] Load model basic config - ok.",
    "@syslog: [ fail ] Load model basic config - fail.",
    "@syslog: [  ok  ] Load model controllers config - ok.",
    "@syslog: [ fail ] Load model controllers config - fail.",
    "@syslog: [  ok  ] Load mras config - ok.",
    "@syslog: [ fail ] Load mras config - fail.",
    "@syslog: [  ok  ] Load ac current sensors config - ok.",
    "@syslog: [ fail ] Load ac current sensors config - fail.",
    "@syslog: [  ok  ] Load angle sensor config - ok.",
    "@syslog: [ fail ] Load angle sensor config - fail.",
    "@syslog: [  ok  ] Load angle sensor calibration data - ok.",
    "@syslog: [ fail ] Load angle sensor calibration data - fail.",

    "@syslog: [  ok  ] Save angle sensor calibration data - ok.",
    "@syslog: [ fail ] Save angle sensor calibration data - fail.",
    
    "@syslog: [  ok  ] Abgle sensor calibration completed successfully.",
    "@syslog: [  ok  ] Abgle sensor calibration terminated."	
};


inline const std::vector<std::string> error_list = {
    "emergency_stop",
    "can_bus_no_conn",
    "dc_undervoltage",
    "dc_overvoltage",
    "dc_overcurrent",
    "module_fault",
    "ac_overcurrent",
    "converter_overtemp",
    "motor_overtemp",
    "vdc_sensor_fault",
    "iac_sensor_fault",
    "eeprom_error",
    "invalid_config",
    "watchdog_timeout",
    "iac_sensor_overcurrent",
};


inline const std::vector<std::string> warning_list = {
    "can_bus_error",
    "can_bus_overrun",
    "can_bus_connection_lost",
    "can_bus_checksum_mismatch",
    "can_bus_counter_freeze",
    "converter_overheat",
    "motor_overheat",
    "flux_weakening",
    "pcbtemp_a_no_conn",
    "pcbtemp_b_no_conn",
    "pcbtemp_c_no_conn",
};


enum class DriveState {
    init,
    standby,
    powerup,
    ready,
    working,
    running,
    tracking,
    powerdown,
    calibrating,
    hwtest,
    stop,
    angsens_cal
};


inline const std::unordered_set<int> drive_state_values {
    std::to_underlying(DriveState::init),
    std::to_underlying(DriveState::standby),
    std::to_underlying(DriveState::powerup),
    std::to_underlying(DriveState::ready),
    std::to_underlying(DriveState::working),
    std::to_underlying(DriveState::running),
    std::to_underlying(DriveState::tracking),
    std::to_underlying(DriveState::powerdown),
    std::to_underlying(DriveState::calibrating),
    std::to_underlying(DriveState::hwtest),
    std::to_underlying(DriveState::stop),
    std::to_underlying(DriveState::angsens_cal)
};


inline const std::unordered_map<DriveState, std::string_view> drive_state_names = {
    {DriveState::init, "init"},
    {DriveState::standby, "standby"},
    {DriveState::powerup, "powerup"},
    {DriveState::ready, "ready"},
    {DriveState::working, "working"},
    {DriveState::running, "running"},
    {DriveState::tracking, "tracking"},
    {DriveState::powerdown, "powerdown"},
    {DriveState::calibrating, "calibrating"},
    {DriveState::hwtest, "hwtest"},
    {DriveState::stop, "stop"},
    {DriveState::angsens_cal, "angsens_cal"}
};


enum class OperatingStatus {
    inoperable,
    working,
    calibrating
};


inline const std::unordered_set<int> opstatus_values = {
    std::to_underlying(OperatingStatus::inoperable),
    std::to_underlying(OperatingStatus::working),
    std::to_underlying(OperatingStatus::calibrating)
};


inline const std::unordered_map<OperatingStatus, std::string_view> opstatus_names = {
    {OperatingStatus::inoperable, "inoperable"},
    {OperatingStatus::working, "working"},
    {OperatingStatus::calibrating, "calibrating"}
};


enum class OperatingMode {
    normal,
    run,
    track,
    hwtest
};


inline const std::unordered_set<int> opmode_values = {
    std::to_underlying(OperatingMode::normal),
    std::to_underlying(OperatingMode::run),
    std::to_underlying(OperatingMode::track),
    std::to_underlying(OperatingMode::hwtest)
};

inline const std::map<OperatingMode, std::string_view> opmode_names = {
    {OperatingMode::normal, "normal"},
    {OperatingMode::run, "run"},
    {OperatingMode::track, "track"},
    {OperatingMode::hwtest, "hwtest"},
};


enum class ControlMode {
    torque,
    speed
};


inline const std::unordered_set<int> ctlmode_values = {
    std::to_underlying(ControlMode::torque),
    std::to_underlying(ControlMode::speed)
};


inline const std::unordered_map<ControlMode, std::string_view> ctlmode_names = {
    {ControlMode::torque, "torque"},
    {ControlMode::speed, "speed"}
};


enum class ControlLoop {
    closed,
    open,
    semiclosed
};


inline const std::unordered_set<int> ctlloop_values = {
    std::to_underlying(ControlLoop::closed),
    std::to_underlying(ControlLoop::open),
    std::to_underlying(ControlLoop::semiclosed)
};


inline const std::unordered_map<ControlLoop, std::string_view> ctlloop_names = {
    {ControlLoop::closed, "closed"},
    {ControlLoop::open, "open"},
    {ControlLoop::semiclosed, "semiclosed"}
};


enum class CalibrationState {
    standby,
    stage1,
    stage2,
    stage3,
    stage4,
    stage5,
    completed
};


inline const std::unordered_set<int> calibration_state_values {
    std::to_underlying(CalibrationState::standby),
    std::to_underlying(CalibrationState::stage1),
    std::to_underlying(CalibrationState::stage2),
    std::to_underlying(CalibrationState::stage3),
    std::to_underlying(CalibrationState::stage4),
    std::to_underlying(CalibrationState::stage5),
    std::to_underlying(CalibrationState::completed),
};


inline const std::unordered_map<CalibrationState, std::string_view> calibration_state_names = {
    {CalibrationState::standby, "standby"},
    {CalibrationState::stage1, "stage1"},
    {CalibrationState::stage2, "stage2"},
    {CalibrationState::stage3, "stage3"},
    {CalibrationState::stage4, "stage4"},
    {CalibrationState::stage5, "stage5"},
    {CalibrationState::completed, "completed"},
};


} // namespace brkdrive
