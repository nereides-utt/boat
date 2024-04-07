#include "drawCell.hpp" // Include the header file for declaration
#include "font_ArialBold.h"
#include "font_Arial.h"

extern ILI9341_t3 tft; // Reference the external 'tft' object

void clearCell(uint8_t x, uint8_t y)
{
    uint8_t height = 60;
    uint8_t width = 79;
    tft.fillRect(x, y, width, height, ILI9341_BLACK);
}

// Implement the rest of the drawing functions here following the same pattern

void drawGauge(uint8_t x, uint8_t y, int16_t backgroundColor, int16_t mainColor)
{
    // Implementation
}

void drawCell(uint8_t x, uint8_t y, int16_t mainColor, float value, String label, String unit)
{
    // Implementation

    u_int8_t height = 60;
    u_int8_t width = 79;
    // u_int8_t padding = 20;
    // u_int8_t borderSize = 2;
    tft.drawRoundRect(x, y, width, height, 5, mainColor);
    tft.drawRoundRect(x, y + 1, width, height, 5, mainColor);
    tft.drawRoundRect(x + 1, y, width, height, 5, mainColor);
    tft.setCursor(x + 8, y + 7);
    tft.setTextColor(mainColor);
    tft.setTextSize(1);
    tft.setFont(Arial_10);
    tft.println(label);
    tft.setCursor(x + 65, y + 7);
    tft.println(unit);
    tft.setCursor(x + 8, y + 30);
    tft.setTextColor(mainColor);
    tft.setTextSize(3);
    tft.setFont(Arial_20_Bold);
    tft.println(value, 1);
}

void drawDoubleCell(uint8_t x, uint8_t y, int16_t mainColor, float value, String label, String unit)
{
    // Implementation

    u_int8_t height = 60;
    u_int8_t width = 159;
    // u_int8_t padding = 20;
    // u_int8_t borderSize = 2;
    tft.drawRoundRect(x, y, width, height, 5, mainColor);
    tft.drawRoundRect(x, y + 1, width, height, 5, mainColor);
    tft.drawRoundRect(x + 1, y, width, height, 5, mainColor);
    tft.setCursor(x + 8, y + 7);
    tft.setTextColor(mainColor);
    tft.setTextSize(1);
    tft.setFont(Arial_10);
    tft.println(label);
    tft.setCursor(x + 130, y + 7);
    tft.println(unit);
    tft.setCursor(x + 8, y + 30);
    tft.setTextColor(mainColor);
    tft.setTextSize(3);
    tft.setFont(Arial_20_Bold);
    tft.println(value, 1);
}

void displaySpeed(uint8_t x, uint8_t y, int16_t mainColor, int value, String label, String unit)
{
    // Implementation

    u_int8_t height = 117;
    u_int16_t width = 319;
    // u_int8_t padding = 20;
    // u_int8_t borderSize = 2;
    tft.drawRoundRect(x, y, width, height, 5, mainColor);
    tft.drawRoundRect(x, y + 1, width, height, 5, mainColor);
    tft.drawRoundRect(x + 1, y, width, height, 5, mainColor);
    tft.setCursor(x + 8, y + 12);
    tft.setTextColor(mainColor);
    tft.setFont(Arial_96_Bold);
    tft.println(value);
    tft.setCursor(x + 160, y + 75);
    tft.setFont(Arial_32_Bold);
    tft.println(unit);
    // tft.setTextSize(1);
    // tft.setFont(Arial_10);
    // tft.println(label);
    // tft.setCursor(x+130, y + 7);
    // tft.println(unit);
    // tft.setCursor(x+8, y + 30);
    // tft.setTextColor(mainColor);  tft.setTextSize(3);
    // tft.setFont(Arial_20_Bold);
    // tft.println(value, 1);
}
