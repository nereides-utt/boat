#ifndef DRAW_CELL_H
#define DRAW_CELL_H

#include "Arduino.h"
#include <ILI9341_t3.h> // Include the ILI9341_t3 library since we're using its types

// Function prototypes
void clearCell(uint8_t x, uint8_t y);
void drawGauge(uint8_t x, uint8_t y, int16_t backgroundColor, int16_t mainColor);
void drawCell(uint8_t x, uint8_t y, int16_t mainColor, float value, String label, String unit);
void drawDoubleCell(uint8_t x, uint8_t y, int16_t mainColor, float value, String label, String unit);
void displaySpeed(uint8_t x, uint8_t y, int16_t mainColor, int value, String label, String unit);

#endif
