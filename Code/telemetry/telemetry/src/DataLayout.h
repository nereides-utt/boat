#ifndef DATALAYOUT_H
#define DATALAYOUT_H

#include <Arduino.h>

#define BASE_TOPIC "nereides"

// Définition de la liste des membres
 //nereides/gps/millis
      //nereides/gps/time
      //nereides/gps/latitude
      //nereides/gps/longitude
     //nereides/gps/vitesse
     //nereides/motor/current_a
     //nereides/motor/voltage_v
     //nereides/motor/rpm
     //nereides/motor/throttle
    //nereides/motor/temp
     //nereides/motor/controller_temp
     //nereides/motor/error_code
     //nereides/motor/controller_status
     //nereides/motor/switch_signals_status
     //nereides/battery/voltage_v
     //nereides/battery/current_a
     //nereides/battery/soc
     //nereides/battery/soh
     //nereides/pac/emergency_stop
     //nereides/batterySE/temp 
     // variable : topic : batterySE_temp 
#define MEMBER_LIST \
    X(String, millis, gps) \
    X(String, time, gps) \
    X(String, latitude, gps) \
    X(String, longitude, gps) \
    X(String, vitesse, gps) \
    X(String, current_a, motor) \
    X(String, voltage_v, motor) \
    X(String, rpm, motor) \
    X(String, throttle, motor) \
    X(String, temp, motor) \
    X(String, controller_temp, motor) \
    X(String, error_code, motor) \
    X(String, controller_status, motor) \
    X(String, switch_signals_status, motor) \
    X(String, voltage_v, battery) \
    X(String, current_a, battery) \
    X(String, soc, battery) \
    X(String, soh, battery) \
    X(String, emergency_stop, pac) \
    X(String, start, pac) \
    X(String, stop, pac) \
    X(String, current_a, pac) \
    X(String, voltage_v, pac) \
    X(String, system_state, pac) \
    X(String, error_flag, pac) \
    X(String, hydrogen_consumption_mgs, pac) \
    X(String, temperature_c, pac) \
    X(String, system_errors, pac) \
    X(String, fan_error, pac) \
    X(String, operation_time, pac) \
    X(String, produced_energy, pac) \
    X(String, total_operation_time, pac) \
    X(String, total_produced_energy, pac) \
    X(String, temp, batterySE)

    

// Utiliser la macro pour déclarer les membres de la structure
#define X(type, name, category) type category##_##name;
struct Data_layout {
    MEMBER_LIST
};
#undef X

// Utiliser la macro pour remplir le tableau nomMesure
#define X(type, name, category) #name,
extern const char* nomMesure[];
#undef X

extern const int nbrMesures;

#define X(type, name, category) #name,
const char* nomMesure[] = {
    MEMBER_LIST
};
#undef X

const int nbrMesures = sizeof(nomMesure) / sizeof(nomMesure[0]);

// Génération des topics MQTT
#define X(type, name, category) extern const char* category##_##name = BASE_TOPIC "/" #category "/" #name;
MEMBER_LIST
#undef X

#endif // DATALAYOUT_H
