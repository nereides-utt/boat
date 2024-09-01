#include <FlexCAN_T4.h>
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1000;
bool DEBUG = true;

//CAN ID: 
#define MOTOR_CONTROLLER_STATUS_1 0x0CF11E05
#define MOTOR_CONTROLLER_STATUS_2 0x0CF11F05
#define BATTERY_STATUS_1 0x12C21020
#define BATTERY_STATUS_2 0x12C21021

// CAN message ID -- Fuel Cell
#define PAC_START_STOP 0x2FC1001
#define PAC_KEEP_ALIVE 0x2FC1000
#define PAC_A_V 0x2FC0001          // every 100ms
#define PAC_SYSTEM_INFOS 0x2FC0000 // every 100ms
#define PAC_SYSTEM_ERRORS 0x2FC0002
#define PAC_TIME_ENERGY_1 0x2FC0003 // every 100ms
#define PAC_TIME_ENERGY_2 0x2FC0004 // every 100ms

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200,SERIAL_8N1);

  can2.begin();
  can2.setBaudRate(250000);
  can2.setMaxMB(16);
  can2.enableFIFO();
  can2.enableFIFOInterrupt();
  can2.onReceive(canSniff);
  can2.mailboxStatus();

  static uint32_t timeout = millis();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

struct data_layout
{
  // Motor
  int motor_current_a;
  int motor_voltage_v;
  int motor_rpm;
  int motor_throttle;
  int motor_temp;
  int motor_controller_temp;
  int motor_error_code;
  int motor_controller_status;
  int motor_switch_signals_status;

  // Battery
  u_int16_t battery_voltage_v;
  int16_t battery_current_a;
  u_int16_t battery_soc;
  u_int16_t battery_soh;
  u_int8_t battery_temp_board;
  u_int8_t battery_temp_pack1;
  u_int8_t battery_temp_pack2;

  // Fuel Cell
  u_int8_t pac_emergency_stop;
  u_int8_t pac_start;
  u_int8_t pac_stop;
  float pac_current_a;
  float pac_voltage_v;
  u_int8_t pac_system_state;
  u_int8_t pac_error_flag;
  u_int8_t pac_hydrogen_consumption_mgs;
  u_int8_t pac_temperature_c;
  u_int8_t pac_system_errors;
  u_int8_t pac_fan_error;
  u_int16_t pac_operation_time;
  u_int16_t pac_produced_energy;
  u_int16_t pac_total_operation_time;
  u_int16_t pac_total_produced_energy;


  char* vitesse; 
};

int indice;
int length;

//dans l'autre version du code faire un new et un old pour comparer à partir de x temps qu'on renvoi la même donnée alors y'a un pb
data_layout data;


void canSniff(const CAN_message_t &msg) {
  // Handle power and errors

/*
  if(msg.id==0){
    data.motor_rpm = 31;     // speed in RPM
    data.motor_current_a = 31;   // current in 0.1A/bit
    data.motor_voltage_v = 31;  // voltage in 0.1V/bit
    data.motor_error_code = 31;
  }*/
  if(msg.id == BATTERY_STATUS_2 && msg.len == 8){
    
    uint16_t raw_temp_board = (uint8_t)msg.buf[4];
    uint16_t raw_temp_pack1 = (uint8_t)msg.buf[5];
    uint16_t raw_temp_pack2 = (uint8_t)msg.buf[6];
    data.battery_temp_board = raw_temp_board - 40;
    data.battery_temp_pack1 = raw_temp_pack1 - 40;
    data.battery_temp_pack2 = raw_temp_pack2 - 40;

  }
  
  if (msg.id == MOTOR_CONTROLLER_STATUS_1 && msg.len == 8)
  {
    data.motor_rpm = ((uint16_t)msg.buf[1] << 8) | msg.buf[0];        // speed in RPM
    data.motor_current_a = ((int16_t)msg.buf[3] << 8) | msg.buf[2];   // current in 0.1A/bit
    data.motor_voltage_v = ((uint16_t)msg.buf[5] << 8) | msg.buf[4];  // voltage in 0.1V/bit
    data.motor_error_code = ((uint16_t)msg.buf[7] << 8) | msg.buf[6]; // error code

    data.motor_current_a / 10;   // Convert to A
    data.motor_voltage_v / 10.0; // Convert to V

    if (DEBUG)
    {
      Serial.print("Speed (RPM): ");
      Serial.println(data.motor_rpm);
      Serial.print("Current (A): ");
      Serial.println(data.motor_current_a);
      Serial.print("Voltage (V): ");
      Serial.println(data.motor_voltage_v);
      Serial.print("Error Code: ");
      Serial.println(data.motor_error_code, HEX);
    }
  }

  // Handle throttle signal and temperatures
  if (msg.id == MOTOR_CONTROLLER_STATUS_2 && msg.len == 8)
  {
    data.motor_throttle = msg.buf[0];
    data.motor_controller_temp = msg.buf[1] - 40; // Convert using offset
    data.motor_temp = msg.buf[2] - 30;            // Convert using offset

    if (DEBUG)
    {
      Serial.print("Throttle Signal (0-255): ");
      Serial.println(data.motor_throttle);
      Serial.print("Controller Temp (°C): ");
      Serial.println(data.motor_controller_temp);
      Serial.print("Motor Temp (°C): ");
      Serial.println(data.motor_temp);
    }
  }

  if (msg.id == BATTERY_STATUS_1 && msg.len >= 8)
  {
    // Replace YOUR_EXPECTED_CAN_ID with the specific CAN ID you're interested in

    data.battery_voltage_v = ((uint16_t)msg.buf[1] << 8) | msg.buf[0];
    data.battery_current_a = ((int16_t)msg.buf[3] << 8) | msg.buf[2];
    data.battery_soc = ((uint16_t)msg.buf[5] << 8) | msg.buf[4];
    data.battery_soh = ((uint16_t)msg.buf[7] << 8) | msg.buf[6];

    int current_mA = -1 * data.battery_current_a / 100;      // Convert to A
    float soc_percentage = data.battery_soc / 100.0;   // Convert to percentage
    float soh_percentage = data.battery_soh / 100.0;   // Convert to percentage
    float voltage_v = data.battery_voltage_v / 1000.0; // Convert to percentage
  }
}

String msg;
String recu;
unsigned long lastHardwareSerialTime=0;
void loop() {
  can2.events();
  unsigned long currentTime = millis();
  if(currentTime-lastSendTime >= sendInterval){
    
    /*msg = "a/" + String(6) +
             "/v/" + String(6) +
             "/r/" + String(6) +
             "/t/" + String(6) +
             "/temp/" + String(6) +
             "/ctemp/" + String(6) +
             "/err/" + String(6) +
             "/stat/" + String(6) +
             "/sig/" + String(6) +

             // Battery
             "/bv/" + String(6) +
             "/ba/" + String(6) +
             "/bsoc/" + String(6) +
             "/bsoh/" + String(6) +

             // Fuel Cell
             "/estop/" + String(6) +
             "/pstart/" + String(6) +
             "/pstop/" + String(6) +
             "/pc/" + String(6) +
             "/pv/" + String(6) +
             "/pstate/" + String(6) +
             "/perr/" + String(6) +
             "/ph2/" + String(6) +
             "/ptemp/" + String(6) +
             "/psyserr/" + String(6) +
             "/pfanerr/" + String(6) +
             "/ptime/" + String(6) +
             "/pprod/" + String(6) +
             "/ptottime/" + String(6) +
             "/ptotprod/" + String(6) 
             + ";";*/
    
    msg = "a/" + String(data.motor_current_a) +
             "/v/" + String((data.motor_voltage_v)/10) +
             "/r/" + String(data.motor_rpm) +
             "/t/" + String(map(data.motor_throttle, 29, 211, 0, 100)) +
             "/temp/" + String(data.motor_temp) +
             "/ctemp/" + String(data.motor_controller_temp) +
             "/err/" + String(data.motor_error_code) +
             "/stat/" + String(data.motor_controller_status) +
             "/sig/" + String(data.motor_switch_signals_status) +

             // Battery
             "/bv/" + String(data.battery_voltage_v) +
             "/ba/" + String(data.battery_current_a) +
             "/bsoc/" + String(data.battery_soc) +
             "/bsoh/" + String(data.battery_soh) +
             "/btb/" + String(data.battery_temp_board) +
             "/btp1/" + String(data.battery_temp_pack1) +
             "/btp2/" + String(data.battery_temp_pack2) + 
    

             // Fuel Cell
             "/estop/" + String(data.pac_emergency_stop) +
             "/pstart/" + String(data.pac_start) +
             "/pstop/" + String(data.pac_stop) +
             "/pc/" + String(data.pac_current_a) +
             "/pv/" + String(data.pac_voltage_v) +
             "/pstate/" + String(data.pac_system_state) +
             "/perr/" + String(data.pac_error_flag) +
             "/ph2/" + String(data.pac_hydrogen_consumption_mgs) +
             "/ptemp/" + String(data.pac_temperature_c) +
             "/psyserr/" + String(data.pac_system_errors) +
             "/pfanerr/" + String(data.pac_fan_error) +
             "/ptime/" + String(data.pac_operation_time) +
             "/pprod/" + String(data.pac_produced_energy) +
             "/ptottime/" + String(data.pac_total_operation_time) +
             "/ptotprod/" + String(data.pac_total_produced_energy) 
             + ";";
    Serial2.print(msg);
    Serial.println(msg);
    lastSendTime=currentTime;
  }
  if (currentTime - lastHardwareSerialTime >= 1000) {
    while (Serial2.available()) {
      recu = Serial2.readStringUntil(';'); 
      int a = recu.toInt();
      CAN_message_t msg;
      msg.id = 0x066;
      for ( uint8_t i = 0; i < recu.length(); i++ ) {
        msg.buf[i] = a;
      }
      can2.write(msg);
    }
    lastHardwareSerialTime = currentTime;
  }
}

//les 0 viennent d'ici? si jamais data.motor_current_a vaut rien encore =0?
String actualiserMsg(){
  String msg = "a/" + String(data.motor_current_a) +
             "/v/" + String(data.motor_voltage_v) +
             "/r/" + String(data.motor_rpm) +
             "/t/" + String(data.motor_throttle) +
             "/temp/" + String(data.motor_temp) +
             "/ctemp/" + String(data.motor_controller_temp) +
             "/err/" + String(data.motor_error_code) +
             "/stat/" + String(data.motor_controller_status) +
             "/sig/" + String(data.motor_switch_signals_status) +

             // Battery
             "/bv/" + String(data.battery_voltage_v) +
             "/ba/" + String(data.battery_current_a) +
             "/bsoc/" + String(data.battery_soc) +
             "/bsoh/" + String(data.battery_soh) +

             // Fuel Cell
             "/estop/" + String(data.pac_emergency_stop) +
             "/pstart/" + String(data.pac_start) +
             "/pstop/" + String(data.pac_stop) +
             "/pc/" + String(data.pac_current_a) +
             "/pv/" + String(data.pac_voltage_v) +
             "/pstate/" + String(data.pac_system_state) +
             "/perr/" + String(data.pac_error_flag) +
             "/ph2/" + String(data.pac_hydrogen_consumption_mgs) +
             "/ptemp/" + String(data.pac_temperature_c) +
             "/psyserr/" + String(data.pac_system_errors) +
             "/pfanerr/" + String(data.pac_fan_error) +
             "/ptime/" + String(data.pac_operation_time) +
             "/pprod/" + String(data.pac_produced_energy) +
             "/ptottime/" + String(data.pac_total_operation_time) +
             "/ptotprod/" + String(data.pac_total_produced_energy) 
             + ";";
          
  return(msg);
}
