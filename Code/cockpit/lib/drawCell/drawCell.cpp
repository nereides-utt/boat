#include "drawCell.hpp" // Include the header file for declaration
#include "font_ArialBold.h"
#include "font_Arial.h"
#include "Arduino.h"
#include "../flickerFree/FlickerFreePrint.h"

extern ILI9341_t3 tft; // Reference the external 'tft' object

void clearCell(uint8_t x, uint8_t y)
{
    uint8_t height = 58;
    uint8_t width = 79;
    tft.fillRect(x, y, width, height, ILI9341_BLACK);
}

// Implement the rest of the drawing functions here following the same pattern

void drawGauge(uint8_t x, uint8_t y, uint8_t percentage, int16_t mainColor)
{
    // Implementation
    u_int16_t height = 10;
    u_int16_t width = 100;
    tft.drawRoundRect(x, y, width, height, 5, mainColor);
    tft.fillRoundRect(x, y, (width * percentage) / 100, height, 5, mainColor);
}

void drawCell(uint8_t x, uint8_t y, uint8_t xSize, uint8_t ySize, int16_t bgColor, int16_t textColor, String label, String unit)
{
    // Implementation

    u_int16_t height = 58;
    u_int16_t width = 79;
    bool isBig = false;

    switch (ySize)
    {
    case 1:
        height = 58;
        break;

    case 2:
        height = 118;
        isBig = true;
        break;

    case 3:
        height = 178;
        isBig = true;
        break;

    case 4:
        height = 239;
        isBig = true;
        break;

    default:
        break;
    }

    switch (xSize)
    {
    case 1:
        width = 79;
        break;

    case 2:
        width = 159;
        isBig = true;
        break;

    case 3:
        width = 239;
        isBig = true;
        break;

    case 4:
        width = 320;
        isBig = true;
        break;

    default:
        break;
    }

    // tft.drawRect(x, y, width, height, mainColor);
    // tft.drawRect(x+1, y + 1, width-2, height-2, mainColor);
    tft.fillRect(x, y, width - 1, height, bgColor);
    tft.setCursor(x + 8, y + 7);
    tft.setTextColor(textColor);
    tft.setTextSize(1);
    tft.setFont(Arial_8_Bold);
    tft.println(label);
}

void displayData(FlickerFreePrint<ILI9341_t3> &data, uint8_t x, uint8_t y, int16_t bgColor, int16_t textColor, float value, u_int8_t decimals, String unit, u_int8_t fontSize = 24, bool boolean = false)
{

    tft.setCursor(x + 8, y + 30);
    tft.setTextColor(textColor);
    tft.setTextSize(3);
    if (fontSize == 24)
    {
        tft.setFont(Arial_24_Bold);
        tft.setCursor(x + 8, y + 30);
    }
    else if (fontSize == 60)
    {
        tft.setFont(Arial_60_Bold);
        tft.setCursor(x + 8, y - 20);
    }

    if (boolean)
    {
        if (value == 1)
        {
            tft.setCursor(x + 14, y + 25);
            tft.println("ON");
        }
        else
        {
            tft.setCursor(x + 6, y + 25);
            tft.println("OFF");
        }
    }
    else
    {
        if (1 == 0)
        {

            // tft.println(String(value) + unit);
            data.setTextColor(textColor, bgColor);
            data.print(value, decimals);
        }
        else
        {

            data.setTextColor(textColor, bgColor);
            data.print(value, decimals);
        }
    }
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
