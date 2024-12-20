/**
 * @file BatteryMonitorTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-17
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "debugUtils.h"

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

int gtl = 0;

int batteryCalibration::getBatteryLevel(int reading)
{
    gtl = reading;
    return -1;
}

int batteryCalibration::getBatteryLevelAutoCalibrated(int reading)
{
    Serial.printf("Reading: %d\n", reading);
    if (gtl!=reading) {
        Serial.println("ERROR: ADC reading at getBatteryLevelAutoCalibrated() differs from reading at getBatteryLevel()");
    }
    return 66;
}

void batteryCalibration::begin()
{
}

void hidImplementation::reportBatteryLevel(int level)
{

}

void notify::lowBattery()
{
}

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("--GO--");
    batteryMonitor::configureForTesting();
    batteryMonitor::begin(
        TEST_BATTERY_READ_ENABLE,
        TEST_BATTERY_READ);
}

void loop()
{
    delay(60 * 1000);
}
