#include "SPI.h"
#include "ILI9341_t3.h"
#include "font_ArialBold.h"
#include "font_Arial.h"
#include "./../lib/drawCell/drawCell.hpp"
#include "./../lib/flickerFree/FlickerFreePrint.h"

#include <FlexCAN_T4.h>
#include <Bounce2.h>

// setup the CAN BUS using the in-built CAN2
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT2_CS 8

#define DEBUG 1

// Define Pushbutton for Teensy
#define BUTTON_1 6
Bounce pushButton = Bounce(BUTTON_1, 10);

// CAN message ID -- Motor
#define MOTOR_CONTROLLER_STATUS_1 0x0CF11E05
#define MOTOR_CONTROLLER_STATUS_2 0x0CF11F05

// CAN message ID -- Battery
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

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
ILI9341_t3 tft2 = ILI9341_t3(TFT2_CS, TFT_DC);

FlickerFreePrint<ILI9341_t3> boat_speed_data(&tft, ILI9341_BLACK, ILI9341_WHITE);

FlickerFreePrint<ILI9341_t3> motor_CAN_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_rpm_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_voltage_v_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_current_a_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_throttle_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_temp_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_controller_temp_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> battery_CAN_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> battery_voltage_v_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> battery_current_a_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> battery_soc_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> battery_soh_data(&tft, ILI9341_BLACK, ILI9341_WHITE);

#define C_BLACK 0x0000
#define C_BLUE 0x001F
#define C_RED 0xF800
#define C_GREEN 0x07E0
#define C_CYAN 0x07FF
#define C_MAGENTA 0xF81F
#define C_YELLOW 0xFFE0
#define C_WHITE 0xFFFF

#define BACKGROUND_COLOR ILI9341_BLACK
#define TEXT_COLOR ILI9341_WHITE

// Define screen grid
static const int SCREEN_WIDTH = 320;
static const int SCREEN_HEIGHT = 240;
static const int X_CENTER = SCREEN_WIDTH / 2;
static const int Y_CENTER = SCREEN_HEIGHT / 2;
static const int ROWS = 4;
static const int COLS = 4;
static const int CELL_WIDTH = SCREEN_WIDTH / COLS;
static const int HALF_CELL_WIDTH = CELL_WIDTH / 2;
static const int CELL_HEIGHT = SCREEN_HEIGHT / ROWS;
static const int HALF_CELL_HEIGHT = CELL_HEIGHT / 2;
static const int COL[] = {0, CELL_WIDTH, CELL_WIDTH * 2, CELL_WIDTH * 3, CELL_WIDTH * 4, CELL_WIDTH * 6, CELL_WIDTH * 7};
static const int ROW[] = {0, 60, 120, 180, CELL_HEIGHT * 4, CELL_HEIGHT * 6, CELL_HEIGHT * 7};
// static const int COL[] = {0, CELL_WIDTH , CELL_WIDTH * 2, CELL_WIDTH * 3, CELL_WIDTH * 4, CELL_WIDTH * 6, CELL_WIDTH * 7};
// static const int ROW[] = {0, CELL_HEIGHT, CELL_HEIGHT * 2, CELL_HEIGHT * 3, CELL_HEIGHT * 4, CELL_HEIGHT * 6, CELL_HEIGHT * 7};

unsigned long last_received_battery_CAN, last_received_motor_CAN, last_received_PAC_CAN;

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
  int motor_controller_temp = -1;
  int motor_error_code = 0;
  int motor_controller_status = 0;
  int motor_switch_signals_status = 0;

  // Battery
  bool battery_CAN = 0;
  u_int16_t battery_voltage_v = -1;
  int16_t battery_current_a = -1;
  u_int16_t battery_soc = -1;
  u_int16_t battery_soh = -1;

  // Fuel Cell
  u_int8_t pac_emergency_stop = 0;
  u_int8_t pac_start = 0;
  u_int8_t pac_stop = 0;
  u_int16_t pac_current_a = 0;
  u_int16_t pac_voltage_v = 0;
  u_int8_t pac_system_state = 0;
  u_int8_t pac_error_flag = 0;
  u_int8_t pac_hydrogen_consumption_mgs = 0;
  u_int8_t pac_temperature_c = 0;
  u_int8_t pac_system_errors = 0;
  u_int8_t pac_fan_error = 0;
  u_int16_t pac_operation_time = 0;
  u_int16_t pac_produced_energy = 0;
  u_int16_t pac_total_operation_time = 0;
  u_int16_t pac_total_produced_energy = 0;

  // Fuel Cell Current Regulator
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
  if (msg.id == 0x666)
  {
    Serial.println("BOAT SPEED RECEIVED");
    new_data.boat_speed = (u_int8_t)msg.buf[0];
  }

  // Handle power and errors
  if (msg.id == MOTOR_CONTROLLER_STATUS_1 && msg.len == 8)
  {
    Serial.println("MOTOR_CONTROLLER_STATUS_1 RECEIVED");
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

  // Handle throttle signal and temperatures
  if (msg.id == MOTOR_CONTROLLER_STATUS_2 && msg.len == 8)
  {
    Serial.println("MOTOR_CONTROLLER_STATUS_2 RECEIVED");
    new_data.motor_throttle = msg.buf[0];
    new_data.motor_controller_temp = msg.buf[1] - 40; // Convert using offset
    new_data.motor_temp = msg.buf[2] - 30;            // Convert using offset
    last_received_motor_CAN = millis();
    new_data.motor_CAN = 1;
    if (DEBUG)
    {
      Serial.print("Throttle Signal (0-255): ");
      Serial.println(new_data.motor_throttle);
      Serial.print("Controller Temp (°C): ");
      Serial.println(new_data.motor_controller_temp);
      Serial.print("Motor Temp (°C): ");
      Serial.println(new_data.motor_temp);
    }
  }

  if (msg.id == BATTERY_STATUS_1 && msg.len >= 8)
  {
    // Replace YOUR_EXPECTED_CAN_ID with the specific CAN ID you're interested in
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

  digitalWrite(LED_BUILTIN, HIGH);
}
// check if CAN data is received
void checkCAN()
{
  // check last received motor CAN message
  if (millis() - last_received_motor_CAN > 1500)
  {
    new_data.motor_CAN = 0;
    new_data.motor_controller_temp = 0;
    new_data.motor_throttle = 0;
    new_data.motor_rpm = 0;
    new_data.motor_temp = 0;
  }

  // check last received battery CAN message
  if (millis() - last_received_battery_CAN > 1500)
  {
    new_data.battery_CAN = 0;
    new_data.battery_current_a = 0;
    new_data.battery_soc = 0;
    new_data.battery_soh = 0;
    new_data.battery_voltage_v = 0;
  }
};

// update the screen value only if they changed
void updateScreen()
{
  // boat data
  if (old_data.boat_speed != new_data.boat_speed)
  {
    old_data.boat_speed = new_data.boat_speed;
    displayData(boat_speed_data, COL[0], ROW[1], BACKGROUND_COLOR, TEXT_COLOR, new_data.boat_speed, 0, "", 60);
  }

  // motor data
  if (old_data.motor_CAN != new_data.motor_CAN)
  {
    old_data.motor_CAN = new_data.motor_CAN;
    displayData(motor_CAN_data, COL[3], ROW[0], BACKGROUND_COLOR, TEXT_COLOR, new_data.motor_CAN, 0, "");
  }

  if (old_data.motor_rpm != new_data.motor_rpm)
  {
    old_data.motor_rpm = new_data.motor_rpm;
    displayData(motor_rpm_data, COL[2], ROW[1], BACKGROUND_COLOR, TEXT_COLOR, new_data.motor_rpm, 0, "");
  }

  if (old_data.motor_temp != new_data.motor_temp)
  {
    old_data.motor_temp = new_data.motor_temp;
    displayData(motor_temp_data, COL[0], ROW[3], BACKGROUND_COLOR, TEXT_COLOR, new_data.motor_temp, 1, "");
  }

  if (old_data.motor_controller_temp != new_data.motor_controller_temp)
  {
    old_data.motor_controller_temp = new_data.motor_controller_temp;
    displayData(motor_controller_temp_data, COL[1], ROW[3], BACKGROUND_COLOR, TEXT_COLOR, new_data.motor_controller_temp, 1, "");
  }

  if (old_data.motor_throttle != new_data.motor_throttle)
  {
    old_data.motor_throttle = new_data.motor_throttle;
    displayData(motor_throttle_data, COL[3], ROW[1], BACKGROUND_COLOR, TEXT_COLOR, new_data.motor_throttle, 0, "");
  }

  // battery data

  if (old_data.battery_CAN != new_data.battery_CAN)
  {
    old_data.battery_CAN = new_data.battery_CAN;
    displayData(battery_CAN_data, COL[2], ROW[0], BACKGROUND_COLOR, TEXT_COLOR, new_data.battery_CAN, 0, "");
    new_data.battery_CAN = 1;
  }

  if (old_data.battery_current_a != new_data.battery_current_a)
  {
    old_data.battery_current_a = new_data.battery_current_a;
    displayData(battery_current_a_data, COL[0], ROW[2], BACKGROUND_COLOR, TEXT_COLOR, new_data.battery_current_a, 0, "");
  }
  if (old_data.battery_voltage_v != new_data.battery_voltage_v)
  {
    old_data.battery_voltage_v = new_data.battery_voltage_v;
    displayData(battery_voltage_v_data, COL[1], ROW[2], BACKGROUND_COLOR, TEXT_COLOR, new_data.battery_voltage_v, 1, "");
  }
  if (old_data.battery_soc != new_data.battery_soc)
  {
    old_data.battery_soc = new_data.battery_soc;
    displayData(battery_soc_data, COL[2], ROW[2], BACKGROUND_COLOR, TEXT_COLOR, new_data.battery_soc, 1, "");
  }

  if (old_data.battery_soh != new_data.battery_soh)
  {
    old_data.battery_soh = new_data.battery_soh;
    displayData(battery_soh_data, COL[3], ROW[2], BACKGROUND_COLOR, TEXT_COLOR, new_data.battery_soh, 0, "");
  }
}
void setup()
{
  can2.begin();
  can2.setBaudRate(250000);
  // can2.setMaxMB(16);
  can2.enableFIFO();
  can2.enableFIFOInterrupt();
  can2.onReceive(canSniff);
  // can2.mailboxStatus();

  // static uint32_t timeout = millis();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Je m'allume");

  tft.begin();
  // Note: you can now set the SPI speed to any value
  // the default value is 30Mhz, but most ILI9341 displays
  // can handle at least 60Mhz and as much as 100Mhz
  //  tft.setClock(60000000);

  tft.setRotation(3);
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);

  tft2.begin();
  // Note: you can now set the SPI speed to any value
  // the default value is 30Mhz, but most ILI9341 displays
  // can handle at least 60Mhz and as much as 100Mhz
  //  tft.setClock(60000000);

  tft2.setRotation(3);
  tft2.fillScreen(ILI9341_WHITE);
  tft2.setTextColor(TEXT_COLOR);
  tft2.setTextSize(2);

  Serial.begin(9600);
  // if (DEBUG)
  // {
  //   while (!Serial)
  //   {
  //     tft.println("Waiting for Arduino Serial Monitor...");
  //   }
  // }

  // read diagnostics (optional but can help debug problems)
  // if (DEBUG)
  // {
  //   uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  //   Serial.print("Display Power Mode: 0x");
  //   Serial.println(x, HEX);
  //   x = tft.readcommand8(ILI9341_RDMADCTL);
  //   Serial.print("MADCTL Mode: 0x");
  //   Serial.println(x, HEX);
  //   x = tft.readcommand8(ILI9341_RDPIXFMT);
  //   Serial.print("Pixel Format: 0x");
  //   Serial.println(x, HEX);
  //   x = tft.readcommand8(ILI9341_RDIMGFMT);
  //   Serial.print("Image Format: 0x");
  //   Serial.println(x, HEX);
  //   x = tft.readcommand8(ILI9341_RDSELFDIAG);
  //   Serial.print("Self Diagnostic: 0x");
  //   Serial.println(x, HEX);
  // }

  drawCell(COL[2], ROW[0], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "CAN BAT.", "");
  drawCell(COL[3], ROW[0], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "CAN MOT", "");
  drawCell(COL[0], ROW[0], 2, 2, BACKGROUND_COLOR, TEXT_COLOR, "SPEED", "");
  drawCell(COL[2], ROW[1], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "RPM", "");
  drawCell(COL[3], ROW[1], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "THROTTLE", "");
  drawCell(COL[0], ROW[2], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "BAT. A", "");
  drawCell(COL[1], ROW[2], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "BAT. V", "");
  drawCell(COL[2], ROW[2], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "SOC", "");
  drawCell(COL[3], ROW[2], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "SOH", "");
  drawCell(COL[0], ROW[3], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "MOTOR T", "%");
  drawCell(COL[1], ROW[3], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "CTRL T", "");
  drawCell(COL[2], ROW[3], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "BAT. T", "");
  drawCell(COL[3], ROW[3], 1, 1, BACKGROUND_COLOR, TEXT_COLOR, "PAC T", "");
  // drawCell(COL[0], ROW[2], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "AMD H2", "");
  // drawCell(COL[1], ROW[2], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "AMD H2", "");

  pinMode(BUTTON_1, INPUT_PULLUP);
  delay(10);
}
int interval = 5000;
unsigned long previousTime = 0, currentTime = 0;
bool flag = false;
int count = 0;

void loop(void)
{
  // generate random values every 5s
  // currentTime = millis();
  // if (currentTime - previousTime > interval)
  // {
  //   if (flag)
  //   {
  //     new_data.motor_rpm = 123;
  //     new_data.motor_throttle = 123;
  //     new_data.battery_current_a = 123;
  //     new_data.battery_voltage_v = 123;
  //     new_data.battery_soc = 123;
  //     new_data.battery_soh = 123;
  //     new_data.motor_temp = 123;
  //     new_data.motor_controller_temp = 123;
  //     flag = !flag;
  //   }
  //   else
  //   {
  //     new_data.motor_rpm = 264;
  //     new_data.motor_throttle = 264;
  //     new_data.battery_current_a = 264;
  //     new_data.battery_voltage_v = 264;
  //     new_data.battery_soc = 264;
  //     new_data.battery_soh = 264;
  //     new_data.motor_temp = 264;
  //     new_data.motor_controller_temp = 264;
  //     flag = !flag;
  //   }
  //   previousTime = currentTime;
  // }
  can2.events();
  updateScreen();
  checkCAN();

  // displayData(motor_rpm_data, COL[0], ROW[0], ILI9341_ORANGE, ILI9341_WHITE, 1248, 0, "");
  // displayData(motor_temp_data, COL[0], ROW[3], ILI9341_ORANGE, ILI9341_WHITE, 21.4, 1, "");
  // displayData(motor_controller_temp_data, COL[3], ROW[3], ILI9341_ORANGE, ILI9341_WHITE, 20.8, 1, "");
}