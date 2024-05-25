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

// CAN message ID
#define MOTOR_CONTROLLER_STATUS_1 0x0CF11E05
#define MOTOR_CONTROLLER_STATUS_2 0x0CF11F05
#define BATTERY_STATUS_1 0x12C21020
#define BATTERY_STATUS_2 0x0CF11F05

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
ILI9341_t3 tft2 = ILI9341_t3(TFT2_CS, TFT_DC);

FlickerFreePrint<ILI9341_t3> motor_rpm_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_voltage_v_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_current_a_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_throttle_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_temp_data(&tft, ILI9341_BLACK, ILI9341_WHITE);
FlickerFreePrint<ILI9341_t3> motor_controller_temp_data(&tft, ILI9341_BLACK, ILI9341_WHITE);

#define C_BLACK 0x0000
#define C_BLUE 0x001F
#define C_RED 0xF800
#define C_GREEN 0x07E0
#define C_CYAN 0x07FF
#define C_MAGENTA 0xF81F
#define C_YELLOW 0xFFE0
#define C_WHITE 0xFFFF

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

// Motor

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
};

data_layout old_data;
data_layout new_data;

void canSniff(const CAN_message_t &msg)
{
  //   if (DEBUG)
  //   {
  //     Serial.print("MB ");
  //     Serial.print(msg.mb);
  //     Serial.print("  OVERRUN: ");
  //     Serial.print(msg.flags.overrun);
  //     Serial.print("  LEN: ");
  //     Serial.print(msg.len);
  //     Serial.print(" EXT: ");
  //     Serial.print(msg.flags.extended);
  //     Serial.print(" TS: ");
  //     Serial.print(msg.timestamp);
  //     Serial.print(" ID: ");
  //     Serial.print(msg.id, HEX);
  //     Serial.print(" Buffer: ");
  //     for (uint8_t i = 0; i < msg.len; i++)
  //     {
  //       Serial.print(msg.buf[i], HEX);
  //       Serial.print(" ");
  //     }
  //     Serial.println();
  //   }

  //   // Handle power and errors
  //   if (msg.id == MOTOR_CONTROLLER_STATUS_1 && msg.len == 8)
  //   {
  //     motor_rpm = ((uint16_t)msg.buf[1] << 8) | msg.buf[0];        // speed in RPM
  //     motor_current_a = ((int16_t)msg.buf[3] << 8) | msg.buf[2];   // current in 0.1A/bit
  //     motor_voltage_v = ((uint16_t)msg.buf[5] << 8) | msg.buf[4];  // voltage in 0.1V/bit
  //     motor_error_code = ((uint16_t)msg.buf[7] << 8) | msg.buf[6]; // error code

  //     motor_current_a / 10;   // Convert to A
  //     motor_voltage_v / 10.0; // Convert to V

  //     if (DEBUG)
  //     {
  //       Serial.print("Speed (RPM): ");
  //       Serial.println(motor_rpm);
  //       Serial.print("Current (A): ");
  //       Serial.println(motor_current_a);
  //       Serial.print("Voltage (V): ");
  //       Serial.println(motor_voltage_v);
  //       Serial.print("Error Code: ");
  //       Serial.println(motor_error_code, HEX);
  //     }
  //   }

  //   // Handle throttle signal and temperatures
  //   if (msg.id == MOTOR_CONTROLLER_STATUS_2 && msg.len == 8)
  //   {
  //     motor_throttle = msg.buf[0];
  //     motor_controller_temp = msg.buf[1] - 40; // Convert using offset
  //     motor_temp = msg.buf[2] - 30;            // Convert using offset

  //     if (DEBUG)
  //     {
  //       Serial.print("Throttle Signal (0-255): ");
  //       Serial.println(motor_throttle);
  //       Serial.print("Controller Temp (°C): ");
  //       Serial.println(motor_controller_temp);
  //       Serial.print("Motor Temp (°C): ");
  //       Serial.println(motor_temp);
  //     }
  //   }

  //   if (msg.id == BATTERY_STATUS_1 && msg.len >= 8)
  //   {
  //     // Replace YOUR_EXPECTED_CAN_ID with the specific CAN ID you're interested in

  //     battery_voltage_v = ((uint16_t)msg.buf[1] << 8) | msg.buf[0];
  //     battery_current_a = ((int16_t)msg.buf[3] << 8) | msg.buf[2];
  //     battery_soc = ((uint16_t)msg.buf[5] << 8) | msg.buf[4];
  //     battery_soh = ((uint16_t)msg.buf[7] << 8) | msg.buf[6];

  //     int current_mA = battery_current_a * 10;      // Convert to mA
  //     float soc_percentage = battery_soc / 100.0;   // Convert to percentage
  //     float soh_percentage = battery_soh / 100.0;   // Convert to percentage
  //     float voltage_v = battery_voltage_v / 1000.0; // Convert to percentage

  //     if (DEBUG)
  //     {
  //       Serial.print("Voltage (mV): ");
  //       Serial.println(voltage_v);
  //       Serial.print("Current (mA): ");
  //       Serial.println(current_mA);
  //       Serial.print("SOC (%): ");
  //       Serial.println(soc_percentage);
  //       Serial.print("SOH (%): ");
  //       Serial.println(soh_percentage);
  //     }
  //   }

  //   digitalWrite(LED_BUILTIN, HIGH);
}

// update the screen value only if they changed
void updateScreen()
{
  if (old_data.motor_rpm != new_data.motor_rpm)
  {
    displayData(motor_rpm_data, COL[0], ROW[0], ILI9341_ORANGE, ILI9341_WHITE, new_data.motor_rpm, 0, "");
    old_data.motor_rpm = new_data.motor_rpm;
    displayData(motor_rpm_data, COL[0], ROW[1], ILI9341_ORANGE, ILI9341_BLACK, new_data.motor_rpm, 0, "");
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
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);

  tft2.begin();
  // Note: you can now set the SPI speed to any value
  // the default value is 30Mhz, but most ILI9341 displays
  // can handle at least 60Mhz and as much as 100Mhz
  //  tft.setClock(60000000);

  tft2.setRotation(3);
  tft2.fillScreen(ILI9341_BLACK);
  tft2.setTextColor(ILI9341_YELLOW);
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
  if (DEBUG)
  {
    uint8_t x = tft.readcommand8(ILI9341_RDMODE);
    Serial.print("Display Power Mode: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDMADCTL);
    Serial.print("MADCTL Mode: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDPIXFMT);
    Serial.print("Pixel Format: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDIMGFMT);
    Serial.print("Image Format: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDSELFDIAG);
    Serial.print("Self Diagnostic: 0x");
    Serial.println(x, HEX);
  }

  drawCell(COL[0], ROW[0], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "RPM", "");
  drawCell(COL[1], ROW[0], 1, 2, ILI9341_ORANGE, ILI9341_WHITE, "RPM", "");
  drawCell(COL[0], ROW[1], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "MOTOR", "");
  drawCell(COL[0], ROW[2], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "MOTOR", "");
  drawCell(COL[0], ROW[3], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "MOTOR", "");
  drawCell(COL[1], ROW[3], 1, 1, ILI9341_PURPLE, ILI9341_WHITE, "PAC", "");
  drawCell(COL[2], ROW[3], 1, 1, ILI9341_GREEN, ILI9341_WHITE, "BATTERY", "");
  drawCell(COL[3], ROW[3], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "CONTROL.", "");
  drawCell(COL[0], ROW[2], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "AMD H2", "");
  drawCell(COL[1], ROW[2], 1, 1, ILI9341_ORANGE, ILI9341_WHITE, "AMD H2", "");
  drawCell(COL[2], ROW[0], 2, 1, ILI9341_ORANGE, ILI9341_WHITE, "BAT. V", " V");
  drawCell(COL[2], ROW[1], 2, 1, ILI9341_ORANGE, ILI9341_WHITE, "BAT. A", " A");
  drawCell(COL[2], ROW[2], 2, 1, ILI9341_ORANGE, ILI9341_WHITE, "PUISSANCE", " W");

  pinMode(BUTTON_1, INPUT_PULLUP);
  delay(10);
}
int interval = 5000;
unsigned long previousTime = 0, currentTime = 0;
bool flag = false;
int count = 0;
void loop(void)
{

  if (pushButton.update())
  {
    if (pushButton.fallingEdge())
    {
      count = count + 1;

      Serial.println(count);
    }
  }
  // generate random values every 5s
  currentTime = millis();
  if (currentTime - previousTime > interval)
  {
    if (flag)
    {
      new_data.motor_rpm = 123;
      flag = !flag;
    }
    else
    {
      new_data.motor_rpm = 125;
      flag = !flag;
    }
    previousTime = currentTime;
  }
  can2.events();
  updateScreen();
  tft2.fillRect(0, 0, 200, 200, ILI9341_CYAN);

  tft2.fillRect(0, 0, 200, 200, ILI9341_RED);

  Serial.println("test");

  // displayData(motor_rpm_data, COL[0], ROW[0], ILI9341_ORANGE, ILI9341_WHITE, 1248, 0, "");
  // displayData(motor_temp_data, COL[0], ROW[3], ILI9341_ORANGE, ILI9341_WHITE, 21.4, 1, "");
  // displayData(motor_controller_temp_data, COL[3], ROW[3], ILI9341_ORANGE, ILI9341_WHITE, 20.8, 1, "");
}