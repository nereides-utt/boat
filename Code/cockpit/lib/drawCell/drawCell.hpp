#ifndef DRAW_CELL_H
#define DRAW_CELL_H

#include "Arduino.h"
#include <ILI9341_t3.h> // Include the ILI9341_t3 library since we're using its types
#include "../flickerFree/FlickerFreePrint.h"
#include <FlickerFreePrint.h>

// Function prototypes
void clearCell(uint8_t x, uint8_t y);
void drawGauge(uint8_t x, uint8_t y, uint8_t percentage, int16_t mainColor);
void drawCell(uint8_t x, uint8_t y, uint8_t xSize, uint8_t ySize, int16_t bgColor, int16_t textColor, String label, String unit);
void drawDoubleCell(uint8_t x, uint8_t y, int16_t mainColor, float value, String label, String unit);
void displaySpeed(uint8_t x, uint8_t y, int16_t mainColor, int value, String label, String unit);
void displayData(FlickerFreePrint<ILI9341_t3> &data, uint8_t x, uint8_t y, int16_t bgColor, int16_t textColor, float value, u_int8_t decimals, String unit, u_int8_t fontSize = 24, bool boolean = false);

#endif
