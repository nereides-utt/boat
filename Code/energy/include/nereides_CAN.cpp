#include "nereides_CAN.h"

void canSniff(const CAN_message_t &msg)
{
    if (DEBUG)
    {
        Serial.print("MB ");
        Serial.print(msg.mb);
        Serial.print("  OVERRUN: ");
        Serial.print(msg.flags.overrun);
        Serial.print("  LEN: ");
        Serial.print(msg.len);
        Serial.print(" EXT: ");
        Serial.print(msg.flags.extended);
        Serial.print(" TS: ");
        Serial.print(msg.timestamp);
        Serial.print(" ID: ");
        Serial.print(msg.id, HEX);
        Serial.print(" Buffer: ");
        for (uint8_t i = 0; i < msg.len; i++)
        {
            Serial.print(msg.buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    // Handle Telemetry - speed
    if (msg.id == 0x666)
    {
        Serial.println("BOAT SPEED RECEIVED");
        new_data.boat_speed = (u_int8_t)msg.buf[0];
    }

    // Handle MC_STATUS_1 - rpm, motor current, motor voltage, error codes
    if (msg.id == MC_STATUS_1 && msg.len == 8)
    {
        Serial.println("MC_STATUS_1 RECEIVED");
        new_data.motor_rpm = ((uint16_t)msg.buf[1] << 8) | msg.buf[0];        // speed in RPM
        new_data.motor_current_a = ((int16_t)msg.buf[3] << 8) | msg.buf[2];   // current in 0.1A/bit
        new_data.motor_voltage_v = ((uint16_t)msg.buf[5] << 8) | msg.buf[4];  // voltage in 0.1V/bit
        new_data.motor_error_code = ((uint16_t)msg.buf[7] << 8) | msg.buf[6]; // error code

        new_data.motor_current_a / 10;   // Convert to A
        new_data.motor_voltage_v / 10.0; // Convert to V
        last_received_motor_CAN = millis();
        new_data.motor_CAN = 1;

        if (DEBUG)
        {
            Serial.print("Speed (RPM): ");
            Serial.println(new_data.motor_rpm);
            Serial.print("Current (A): ");
            Serial.println(new_data.motor_current_a);
            Serial.print("Voltage (V): ");
            Serial.println(new_data.motor_voltage_v);
            Serial.print("Error Code: ");
            Serial.println(new_data.motor_error_code, HEX);
        }
    }

    // Handle MC_STATUS_2 - throttle signal, controller temperature, motor temperature
    if (msg.id == MC_STATUS_2 && msg.len == 8)
    {
        Serial.println("MC_STATUS_2 RECEIVED");
        new_data.motor_throttle = msg.buf[0];
        new_data.MC_temp = msg.buf[1] - 40;    // Convert using offset
        new_data.motor_temp = msg.buf[2] - 30; // Convert using offset
        last_received_motor_CAN = millis();
        new_data.motor_CAN = 1;
        if (DEBUG)
        {
            Serial.print("Throttle Signal (0-255): ");
            Serial.println(new_data.motor_throttle);
            Serial.print("Controller Temp (°C): ");
            Serial.println(new_data.MC_temp);
            Serial.print("Motor Temp (°C): ");
            Serial.println(new_data.motor_temp);
        }
    }

    // Handle BATTERY_1_STATUS_1 - current, voltage, soc, soh
    if (msg.id == BATTERY_STATUS_1 && msg.len >= 8)
    {
        Serial.println("BATTERY _STATUS_1 RECEIVED");
        new_data.battery_voltage_v = ((uint16_t)msg.buf[1] << 8) | msg.buf[0];
        new_data.battery_current_a = ((int16_t)msg.buf[3] << 8) | msg.buf[2];
        new_data.battery_soc = ((uint16_t)msg.buf[5] << 8) | msg.buf[4];
        new_data.battery_soh = ((uint16_t)msg.buf[7] << 8) | msg.buf[6];

        new_data.battery_current_a = new_data.battery_current_a * 10;     // Convert to mA
        new_data.battery_soc = new_data.battery_soc / 100.0;              // Convert to percentage
        new_data.battery_soh = new_data.battery_soh / 100.0;              // Convert to percentage
        new_data.battery_voltage_v = new_data.battery_voltage_v / 1000.0; // Convert to percentage
        last_received_battery_CAN = millis();
        new_data.battery_CAN = 1;

        if (DEBUG)
        {
            Serial.print("Voltage (mV): ");
            Serial.println(new_data.battery_voltage_v);
            Serial.print("Current (mA): ");
            Serial.println(new_data.battery_current_a);
            Serial.print("SOC (%): ");
            Serial.println(new_data.battery_soc);
            Serial.print("SOH (%): ");
            Serial.println(new_data.battery_soh);
        }
    }

    // Handle FUEL CELL - current, voltage
    if (msg.id == FC_A_V)
    {
        Serial.println("FC_A_V RECEIVED");

        new_data.fc_current_a = ((uint16_t)msg.buf[0] << 8) | msg.buf[3];
        new_data.fc_voltage_v = ((uint16_t)msg.buf[4] << 8) | msg.buf[7];

        new_data.fc_current_a = new_data.fc_current_a * 0.01; // Convert to A
        new_data.fc_voltage_v = new_data.fc_voltage_v * 0.01; // Convert to V

        last_received_FC_CAN = millis();
        new_data.fc_CAN = 1;

        if (DEBUG)
        {
            Serial.print("FC Current (A): ");
            Serial.println(new_data.fc_current_a);
            Serial.print("FC Voltage (V): ");
            Serial.println(new_data.fc_voltage_v);
        }
    }

    // Handle FUEL CELL - system_state, error_flag, h2_consumption, temperature
    if (msg.id == FC_SYSTEM_INFOS)
    {
        Serial.println("FC_INFOS RECEIVED");

        new_data.fc_system_state = msg.buf[0];
        new_data.fc_error_flag = msg.buf[1];
        new_data.fc_hydrogen_consumption_mgs = msg.buf[2];
        new_data.fc_temperature_c = msg.buf[3]; // 1 bit <-> 1°C

        last_received_FC_CAN = millis();
        new_data.fc_CAN = 1;

        if (DEBUG)
        {
            Serial.print("FC System State : ");
            switch (new_data.fc_system_state)
            {
            case 10:
                Serial.println("INITIALISATION");
                break;
            case 20:
                Serial.println("AUTOCHECK");
                break;
            case 30:
                Serial.println("STANDBY");
                break;
            case 40:
                Serial.println("MAINTENANCE");
                break;
            case 50:
                Serial.println("DÉMARRAGE - PURGE");
                break;
            case 51:
                Serial.println("DÉMARRAGE - VÉRIFICATION TENSION");
                break;
            case 52:
                Serial.println("DÉMARRAGE - WARM UP");
                break;
            case 100:
                Serial.println("EN OPÉRATION");
                break;
            case 200:
                Serial.println("POWER DOWN");
                break;
            case 201:
                Serial.println("PURGE");
                break;
            case 202:
                Serial.println("H2 STOP");
                break;
            case 203:
                Serial.println("PURGE STOP");
                break;
            case 204:
                Serial.println("SAUVEGARDE EEPROM");
                break;
            case 250:
                Serial.println("ARRÊT URGENT");
                break;
            }
            Serial.print("FC ERROR : ");
            switch (new_data.fc_error_flag)
            {
            case 0:
                Serial.println("NO ERROR !");
                break;
            case 1:
                Serial.println("CRITICAL ERROR !");
                break;
            case 2:
                Serial.println("MINOR ERROR !");
                break;
            case 3:
                Serial.println("CRITICAL, MINOR ERROR !");
                break;
            case 4:
                Serial.println("H2 ERROR !");
                break;
            case 5:
                Serial.println("CRITICAL, H2 ERROR !");
                break;
            case 6:
                Serial.println("MINOR, H2 ERROR !");
                break;
            case 7:
                Serial.println("CRITICAL, MINOR, H2 ERROR !");
                break;
            }
            Serial.print("FC Hydrogen Consumption (mg/s) : ");
            Serial.println(new_data.fc_hydrogen_consumption_mgs);
            Serial.print("FC Temperature : ");
            Serial.println(new_data.fc_temperature_c);
        }
    }

    // Handle FUEL CELL - system_errors, fan_error
    if (msg.id == FC_SYSTEM_INFOS)
    {
        Serial.println("FC_ERRORS RECEIVED");

        new_data.fc_system_errors = msg.buf[0];
        new_data.fc_fan_error = msg.buf[1];

        last_received_FC_CAN = millis();
        new_data.fc_CAN = 1;

        if (DEBUG)
        {
            Serial.print("FC System Error Code : ");
            Serial.println(new_data.fc_system_errors);
            Serial.print("FC System Fan Error Code : ");
            Serial.println(new_data.fc_fan_error);
        }
    }

    // Handle FUEL CELL - operation_time, produced_energy
    if (msg.id == FC_TIME_ENERGY_1)
    {
        Serial.println("FC_TIME_ENERGY_1 RECEIVED");

        new_data.fc_operation_time = ((uint16_t)msg.buf[0] << 8) | msg.buf[1];  // operation time in (Minutes)
        new_data.fc_produced_energy = ((uint16_t)msg.buf[2] << 8) | msg.buf[3]; // produced energy (Wh)
        last_received_FC_CAN = millis();
        new_data.fc_CAN = 1;

        if (DEBUG)
        {
            Serial.print("FC Operation Time : ");
            Serial.println(new_data.fc_operation_time);
            Serial.print("FC Produced Energy : ");
            Serial.println(new_data.fc_produced_energy);
        }
    }

    // Handle FUEL CELL - total_operation_time, total_produced_energy
    if (msg.id == FC_TIME_ENERGY_2)
    {
        Serial.println("FC_TIME_ENERGY_2 RECEIVED");

        new_data.fc_total_operation_time = ((uint16_t)msg.buf[0] << 8) | msg.buf[3];  // operation time in (Minutes)
        new_data.fc_total_produced_energy = ((uint16_t)msg.buf[4] << 8) | msg.buf[7]; // produced energy (Wh)
        last_received_FC_CAN = millis();
        new_data.fc_CAN = 1;

        if (DEBUG)
        {
            Serial.print("FC Total Operation Time : ");
            Serial.println(new_data.fc_total_operation_time);
            Serial.print("FC TotalProduced Energy : ");
            Serial.println(new_data.fc_total_produced_energy);
        }
    }

    // Handle FUEL CELL CURRENT REGULATOR - current_setting_request, current, temperature, temperature_error
    if (msg.id == FCCR_INFOS)

    {
        Serial.println("FCCR_INFOS RECEIVED");

        new_data.fccr_current_setting_request = msg.buf[0];                  // 0-55 (A)
        new_data.fccr_current = msg.buf[1];                                  // 0-55 (A)
        new_data.fccr_temperature = (u_int16_t)msg.buf[2] << 8 | msg.buf[3]; // temperature (°C)
        new_data.fccr_temperature_error = msg.buf[4];
        last_received_FCCR_CAN = millis();
        new_data.fccr_CAN = 1;

        if (DEBUG)
        {
            Serial.print("FCCR Current Setting Request ");
            Serial.println(new_data.fccr_current_setting_request);
            Serial.print("FCCR Current : ");
            Serial.println(new_data.fccr_current);
            Serial.print("FCCR temperature : ");
            Serial.println(new_data.fccr_temperature);
            Serial.print("FCCR Temperature Error : ");
            Serial.println(new_data.fccr_temperature_error);
        }
    }

    // Handle FUEL CELL CURRENT REGULATOR - current_setting_request, current, temperature, temperature_error
    if (msg.id == FCCR_CONFIG_INFOS)

    {
        Serial.println("FCCR_CONFIG_INFOS RECEIVED");

        new_data.fccr_baudrate = msg.buf[0]; // 1:125kbps, 2:250kbps, 3:500kbps
        new_data.fccr_number_DCDC = msg.buf[1];
        new_data.fccr_ramp_setting = msg.buf[2];                          //! Possibilité erreur doc!!
        new_data.fccr_current_limit_setting = msg.buf[3];                 // Current limit (A)
        new_data.fccr_max_current = ((u_int16_t)msg.buf[4]) | msg.buf[5]; // Max Current Limit (A)
        new_data.fccr_software_revision = msg.buf[6];                     // Current limit (A)
        last_received_FCCR_CAN = millis();
        new_data.fccr_CAN = 1;

        if (DEBUG)
        {
            Serial.print("FCCR Baudrate : ");
            Serial.println(new_data.fccr_baudrate);
            Serial.print("FCCR Number DCDC : ");
            Serial.println(new_data.fccr_number_DCDC);
            Serial.print("FCCR Ramp Setting : ");
            Serial.println(new_data.fccr_ramp_setting);
            Serial.print("FCCR Current Limit Setting : ");
            Serial.println(new_data.fccr_current_limit_setting);
            Serial.print("FCCR Max Current : ");
            Serial.println(new_data.fccr_max_current);
            Serial.print("FCCR Software Revision : ");
            Serial.println(new_data.fccr_software_revision);
        }
    }
}