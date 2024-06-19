#include <FlexCAN_T4.h>

// Debug Flag
#define DEBUG true

// setup the CAN BUS using the in-built CAN2
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

// CAN message ID -- Motor
#define MC_STATUS_1 0x0CF11E05 // <- every 1000ms
#define MC_STATUS_2 0x0CF11F05 // <- every 1000ms

// CAN message ID -- Battery
#define BATTERY_STATUS_1 0x12C21020 // <- every 1000ms
#define BATTERY_STATUS_2 0x12C21021 // <- every 1000ms

// CAN message ID -- Fuel Cell
#define FC_START_STOP 0x2FC1001    //->
#define FC_KEEP_ALIVE 0x2FC1000    //->
#define FC_A_V 0x2FC0001           // <- every 100ms
#define FC_SYSTEM_INFOS 0x2FC0000  // <- every 100ms
#define FC_SYSTEM_ERRORS 0x2FC0002 // <- if errors
#define FC_TIME_ENERGY_1 0x2FC0003 // <- every 100ms
#define FC_TIME_ENERGY_2 0x2FC0004 // <- every 100ms

// CAN message ID -- Fuel Cell Current Regulator
#define FCCR_CONTROL 0x050          // ->
#define FCCR_INFOS 0x051            // <-
#define FCCR_CONFIG_INFOS 0x2FC0001 // <-

struct data_layout
{
    // boat
    u_int8_t boat_speed = -1;

    // Motor
    bool motor_CAN = 0;
    int motor_current_a = -1;
    int motor_voltage_v = -1;
    int motor_rpm = -1;
    int motor_throttle = -1;
    int motor_temp = -1;
    int MC_temp = -1;
    int motor_error_code = 0;
    int MC_status = 0;
    int motor_switch_signals_status = 0;

    // Battery
    bool battery_CAN = 0;
    u_int16_t battery_voltage_v = -1;
    int16_t battery_current_a = -1;
    u_int16_t battery_soc = -1;
    u_int16_t battery_soh = -1;

    // Fuel Cell
    bool fc_CAN = 0;
    u_int8_t fc_emergency_stop = 0;
    u_int8_t fc_start = 0;
    u_int8_t fc_stop = 0;
    u_int16_t fc_current_a = 0;
    u_int16_t fc_voltage_v = 0;
    u_int8_t fc_system_state = 0;
    u_int8_t fc_error_flag = 0;
    u_int8_t fc_hydrogen_consumption_mgs = 0;
    u_int8_t fc_temperature_c = 0;
    u_int8_t fc_system_errors = 0;
    u_int8_t fc_fan_error = 0;
    u_int16_t fc_operation_time = 0;
    u_int16_t fc_produced_energy = 0;
    u_int16_t fc_total_operation_time = 0;
    u_int16_t fc_total_produced_energy = 0;

    // Fuel Cell Current Regulator
    bool fccr_CAN = 0;
    u_int16_t fccr_current_command = 0;
    u_int8_t fccr_current_setting_request = 0;
    u_int8_t fccr_current = 0;
    u_int16_t fccr_temperature = 0;
    bool fccr_temperature_error = false;
    u_int8_t fccr_baudrate = 0;
    u_int8_t fccr_number_DCDC = 0;
    u_int8_t fccr_ramp_setting = 0;
    u_int8_t fccr_current_limit_setting = 0;
    u_int16_t fccr_max_current = 0;
    u_int8_t fccr_software_revision = 0;
};

data_layout old_data;
data_layout new_data;

unsigned long last_received_FC_CAN, last_received_FCCR_CAN, last_received_battery_CAN, last_received_motor_CAN;
