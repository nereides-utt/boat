#include <Arduino.h>

#include<stdio.h>
#include <stdint.h>
#include "./../libraries/FlexCAN_T4/FlexCAN_T4.h"
#include <FlexCAN_T4.h>


// setup the CAN BUS using the in-built CAN2
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

static const uint8_t PAC_emergency_stop = 1;
static const uint8_t PAC_stop = 1;
static const uint8_t PAC_start = 1;


#define PAC_KEEP_ALIVE 0x2fc1000
#define PAC_ORDER 0x2fc1001

#define PAC_INFO_SYSTEM 0x2fc0001
#define PAC_STATE_SYSTEM 0x2fc0000
#define PAC_ERROR_SYSTEM 0x2fc0002
#define PAC_TIME_ENERGY1 0x2fc0003
#define PAC_TIME_ENERGY2 0x2fc0004


typedef struct PAC_errors
{
  uint8_t erreur_interne;
  uint8_t tension_pile;
  uint8_t courant_pile1;
  uint8_t courant_pile2;
  uint8_t temp_pile;
  uint8_t fuite_hydrogene;
  uint8_t pression_hydrogene;
  uint8_t tension_auxiliaire;
  uint8_t erreur_ventilateur;
}errors;

 typedef struct PAC_FLAGS
{
  uint8_t erreur_critique;
  uint8_t erreur_mineure;
  uint8_t alerte_h2;
}PAC_FLAGS;

 struct PAC_data
{
  uint16_t amperage;
  uint16_t voltage;
  uint8_t system_state;
  PAC_FLAGS flag;
  uint8_t hydrogen_conso;
  uint8_t temp;
  errors error;
  uint16_t time1;
  uint16_t energy_prod1;
  uint16_t time2;
  uint16_t energy_prod2;
}PAC_data;


void canSniff(const CAN_message_t &msg)
{

  if (msg.len == 8)
  {
    switch(msg.id)
    {

    case PAC_INFO_SYSTEM:
    PAC_data.amperage = msg.buf[0] << 8 | msg.buf[3];
    PAC_data.voltage = msg.buf[4] << 8 | msg.buf[7];
    break;  

    case PAC_STATE_SYSTEM:
    PAC_data.system_state = msg.buf[0];
    
    PAC_data.flag.erreur_critique = msg.buf[1]&1;
    PAC_data.flag.erreur_mineure = (msg.buf[1]>>1)&1;
    PAC_data.flag.alerte_h2 = (msg.buf[1]>>2)&1;

    PAC_data.hydrogen_conso_instant = msg.buf[2];
    PAC_data.temp = msg.buf[3];
    break;

    case PAC_ERROR_SYSTEM:
    PAC_data.error.erreur_interne = msg.buf[0]&1;
    PAC_data.error.tension_pile = (msg.buf[0]>>1)&1;
    PAC_data.error.courant_pile1 = (msg.buf[0]>>2)&1;
    PAC_data.error.courant_pile2 = (msg.buf[0]>>3)&1;
    PAC_data.error.temp_pile = (msg.buf[0]>>4)&1;
    PAC_data.error.fuite_hydrogene = (msg.buf[0]>>5)&1;
    PAC_data.error.pression_hydrogene = (msg.buf[0]>>6)&1;
    PAC_data.error.tension_auxiliaire = (msg.buf[0]>>7)&1;
    
    PAC_data.error.erreur_ventilateur = msg.buf[1];
    break;

    case PAC_TIME_ENERGY1:
    PAC_data.time1 = msg.buf[0] << 8 | msg.buf[1];
    PAC_data.energy_prod1 = msg.buf[2] << 8 | msg.buf[3];

    break;

    case PAC_TIME_ENERGY2:  
    PAC_data.time2 = msg.buf[0] << 8 | msg.buf[3];
    PAC_data.energy_prod2 = msg.buf[4] << 8 | msg.buf[7];




    default:

    break;


    }
  }
}





 
void loop(void)
{
  can2.events();
    
}


