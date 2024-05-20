#include "SPI.h"
#include "ILI9341_t3.h"
#include "font_ArialBold.h"
#include "font_Arial.h"
#include "./../lib/drawCell/drawCell.hpp"

#include <FlexCAN_T4.h>

// setup the CAN BUS using the in-built CAN2
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

static const int SCREEN_WIDTH = 320;
static const int SCREEN_HEIGHT = 240;
static const int X_CENTER = SCREEN_WIDTH / 2;
static const int Y_CENTER = SCREEN_HEIGHT / 2;
static const int ROWS = 4;
static const int COLS = 4;
static const int CELL_WIDTH = SCREEN_WIDTH / COLS;
static const int HALF_CELL_WIDTH = CELL_WIDTH / 2;
static const int CELL_HIGHT = SCREEN_HEIGHT / ROWS;
static const int HALF_CELL_HIGHT = CELL_HIGHT / 2;
static const int COL[] = {0, CELL_WIDTH, CELL_WIDTH * 2, CELL_WIDTH * 3, CELL_WIDTH * 4, CELL_WIDTH * 6, CELL_WIDTH * 7};
static const int ROW[] = {0, CELL_HIGHT, CELL_HIGHT * 2 - 2, CELL_HIGHT * 3 - 1, CELL_HIGHT * 4, CELL_HIGHT * 6, CELL_HIGHT * 7};

void canSniff(const CAN_message_t &msg)
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

  if (msg.id == 0x12C21020 && msg.len >= 8)
  {
    // Replace YOUR_EXPECTED_CAN_ID with the specific CAN ID you're interested in

    uint16_t voltage = ((uint16_t)msg.buf[1] << 8) | msg.buf[0];
    int16_t current = ((int16_t)msg.buf[3] << 8) | msg.buf[2];
    uint16_t soc = ((uint16_t)msg.buf[5] << 8) | msg.buf[4];
    uint16_t soh = ((uint16_t)msg.buf[7] << 8) | msg.buf[6];

    int current_mA = current * 10;      // Convert to mA
    float soc_percentage = soc / 100.0; // Convert to percentage
    float soh_percentage = soh / 100.0; // Convert to percentage
    float voltage_v = voltage / 1000.0; // Convert to percentage

    clearCell(COL[0], ROW[3]);
    drawCell(COL[0], ROW[3], 1, 1, ILI9341_GREEN, soc_percentage, "CHG", "%");
    clearCell(COL[1], ROW[3]);
    drawCell(COL[1], ROW[3], 1, 1, ILI9341_GREEN, current_mA, "AMP", "mA");
    clearCell(COL[2], ROW[3]);
    drawCell(COL[2], ROW[3], 1, 1, ILI9341_GREEN, soh_percentage, "SOH", "%");
    clearCell(COL[3], ROW[3]);
    drawCell(COL[3], ROW[3], 1, 1, ILI9341_GREEN, voltage_v, "VLT", "V");

    Serial.print("Voltage (mV): ");
    Serial.println(voltage);
    Serial.print("Current (mA): ");
    Serial.println(current_mA);
    Serial.print("SOC (%): ");
    Serial.println(soc_percentage);
    Serial.print("SOH (%): ");
    Serial.println(soh_percentage);
  }

  digitalWrite(LED_BUILTIN, HIGH);
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
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println("Waiting for Arduino Serial Monitor...");

  Serial.begin(9600);
  // while (!Serial)
  ; // wait for Arduino Serial Monitor
  Serial.println("ILI9341 Test!");

  // read diagnostics (optional but can help debug problems)
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

  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);

  drawCell(COL[0], ROW[3], 1, 1, ILI9341_GREEN, 21.8, "MOT", "°C");
  drawCell(COL[1], ROW[3], 1, 1, ILI9341_GREEN, 21.8, "PAC", "°C");
  drawCell(COL[2], ROW[3], 1, 1, ILI9341_RED, 21.8, "BT1", "°C");
  drawCell(COL[3], ROW[3], 1, 1, ILI9341_YELLOW, 21.8, "BAT2", "°C");
  // drawCell(COL[0], ROW[2], 2, 1, ILI9341_YELLOW, 21.8, "ENG", "%", true);

  // drawCell(COL[2], ROW[2],2,2, ILI9341_GREEN, 21.8, "MOT", "°C");
  //  drawCell(COL[1], ROW[3], ILI9341_GREEN, 31.7, "PAC", "°C");
  //  drawCell(COL[2], ROW[3], ILI9341_GREEN, 38.1, "BT1", "°C");
  //  drawCell(COL[3], ROW[3], ILI9341_GREEN, 27.4, "BT2", "°C");
  //  drawCell(COL[3], ROW[2], ILI9341_RED, 27, "BT2", "A");
  //  drawCell(COL[3], ROW[1], ILI9341_RED, 27, "BT2", "A");
  //  drawCell(COL[3], ROW[0], ILI9341_PURPLE, 27, "BT2", "A");
  //  drawCell(COL[0], ROW[0], ILI9341_ORANGE, 48.1, "BT2", "°V");
  //  drawDoubleCell(0, 118, ILI9341_YELLOW, 27.4, "BT2", "°C");
  // displaySpeed(COL[0], ROW[0], ILI9341_YELLOW, 27, "BT2", "km/h");
  delay(3000);

  Serial.println(F("Done!"));
}

void loop(void)
{

  can2.events();
}