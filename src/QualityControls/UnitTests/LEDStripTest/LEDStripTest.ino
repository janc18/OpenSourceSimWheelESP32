/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-24
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "debugUtils.h"
#include "LedStrip.h"

#define LED_COUNT 8

LEDStrip *strip;

void setup()
{
    Serial.begin(115200);
    strip = new LEDStrip(TEST_D_OUT, LED_COUNT, true);
    strip->brightness(16);
    // strip = new LEDStrip(LED_PIN, LED_COUNT, PIXEL_WS2811);
    Serial.println("--GO--");
}

void loop()
{
    Serial.println("Go red");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 255, 0, 0);
    strip->show();
    delay(2000);
    Serial.println("Go green");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 0, 255, 0);
    strip->show();
    delay(2000);
    Serial.println("Go blue");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 0, 0, 255);
    strip->show();
    delay(2000);
    Serial.println("Go white");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 255, 255, 255);
    strip->show();
    delay(2000);
    Serial.println("Go purple");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 128, 0, 128);
    strip->show();
    delay(2000);
    Serial.println("Go orange");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 255, 65, 0);
    strip->show();
    delay(2000);
    Serial.println("Go orange dimmer");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 64, 16, 0);
    strip->show();
    delay(2000);
    Serial.println("Go off");
    strip->clear();
    strip->show();
    delay(2000);
    Serial.println("rainbow");
    strip->pixelRGB(0, 0xEE82EE);
    strip->pixelRGB(1, 0x4B0082);
    strip->pixelRGB(2, 0x0000FF);
    strip->pixelRGB(3, 0x008000);
    strip->pixelRGB(4, 0xFFFF00);
    strip->pixelRGB(5, 0xFFA500);
    strip->pixelRGB(6, 0xFF0000);
    strip->show();
    delay(4000);
}