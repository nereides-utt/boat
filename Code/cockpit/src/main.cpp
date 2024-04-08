#include "SPI.h"
#include "ILI9341_t3.h"
#include "font_ArialBold.h"
#include "font_Arial.h"
#include "./../lib/drawCell/drawCell.hpp"

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
//modif
void setup()
{
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
  while (!Serial)
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
  drawCell(COL[0], ROW[0], 2, 1, ILI9341_YELLOW, 21.8, "ENG", "%");

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
}


/*
#include <FlexCAN_T4.h>
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
static uint32_t timeout = millis();
void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
  
  can2.begin();
  can2.setBaudRate(1000000);
  can2.setMaxMB(16);
  can2.enableFIFO();
  can2.enableFIFOInterrupt();
  can2.onReceive(canSniff);
  can2.mailboxStatus();
  }
  
  
  void canSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();

  digitalWrite(LED_BUILTIN, HIGH);
}
  */