/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheel.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define LCLUTCH 1
#define RCLUTCH 2
#define ALT 3
#define CMD 4
#define CYCLE_CLUTCH 5
#define CYCLE_ALT 6
#define UP 7
#define DOWN 8
#define LEFT 9
#define RIGHT 10
#define CYCLE_DPAD 11
#define OTHER 20
#define OTHER2 21
#define OTHER3 22

#define OTHER_MAP 126
#define OTHER_MAP_ALT 30

#define COMBINATION_CYCLE_CLUTCH {CMD, CYCLE_CLUTCH}
#define COMBINATION_CYCLE_ALT {CMD, CYCLE_ALT}
#define COMBINATION_SELECT_CLUTCH_F {CMD, UP}
#define COMBINATION_SELECT_ALT_F {(CMD), (DOWN)}
#define COMBINATION_SELECT_AXIS_F {(CMD), (LEFT)}
#define COMBINATION_SELECT_BUTTON_F {(CMD), (RIGHT)}
#define COMBINATION_SELECT_LC_LEFT_F {(CMD), (OTHER2)}
#define COMBINATION_SELECT_LC_RIGHT_F {(CMD), (OTHER3)}
#define COMBINATION_SECURITY_LOCK {(CMD), (OTHER), (UP)}

#define BMP_CYCLE_CLUTCH BITMAP(CMD) | BITMAP(CYCLE_CLUTCH)
#define BMP_CYCLE_ALT BITMAP(CMD) | BITMAP(CYCLE_ALT)
#define BMP_SELECT_CLUTCH_F BITMAP(CMD) | BITMAP(UP)
#define BMP_SELECT_ALT_F BITMAP(CMD) | BITMAP(DOWN)
#define BMP_SELECT_AXIS_F BITMAP(CMD) | BITMAP(LEFT)
#define BMP_SELECT_BUTTON_F BITMAP(CMD) | BITMAP(RIGHT)
#define BMP_SELECT_LC_LEFT_F BITMAP(CMD) | BITMAP(OTHER2)
#define BMP_SELECT_LC_RIGHT_F BITMAP(CMD) | BITMAP(OTHER3)

#define BMP_OTHER_MAP_LOW 0ULL
#define BMP_OTHER_MAP_HIGH BITMAP(OTHER_MAP - 64)
#define BMP_OTHER_MAP_ALT_LOW BITMAP(OTHER_MAP_ALT)
#define BMP_OTHER_MAP_ALT_HIGH 0ULL

#define ALT_B BITMAP(ALT)
#define UP_B BITMAP(UP)
#define DOWN_B BITMAP(DOWN)
#define LEFT_B BITMAP(LEFT)
#define RIGHT_B BITMAP(RIGHT)

//------------------------------------------------------------------
// Auxiliary: input simulator
//------------------------------------------------------------------

class InputSimulator
{
public:
    void push(inputNumber_t btnNumber);
    void pushSeveral(inputBitmap_t bmp);
    void release(inputNumber_t btnNumber = UNSPECIFIED_INPUT_NUMBER);
    void repeat();
    void axis(clutchValue_t left, clutchValue_t right);

    bool hasAnalogAxes = true;

private:
    inputBitmap_t prevRawInput = 0ULL;
    inputBitmap_t rawInput = 0ULL;
    inputBitmap_t rawInputChanges = 0ULL;
    clutchValue_t leftAxis = CLUTCH_NONE_VALUE;
    clutchValue_t rightAxis = CLUTCH_NONE_VALUE;
} input;

void InputSimulator::push(inputNumber_t btnNumber)
{
    rawInput |= BITMAP(btnNumber);
    repeat();
}

void InputSimulator::pushSeveral(inputBitmap_t bmp)
{
    rawInput |= bmp;
    repeat();
}

void InputSimulator::release(inputNumber_t btnNumber)
{
    if (btnNumber == UNSPECIFIED_INPUT_NUMBER)
        rawInput = 0ULL;
    else
        rawInput &= ~BITMAP(btnNumber);
    repeat();
}

void InputSimulator::repeat()
{
    inputHub::onRawInput(
        rawInput,
        (prevRawInput ^ rawInput),
        leftAxis,
        rightAxis,
        hasAnalogAxes);
    prevRawInput = rawInput;
}

void InputSimulator::axis(clutchValue_t left, clutchValue_t right)
{
    leftAxis = left;
    rightAxis = right;
    repeat();
}

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

uint8_t currentPOV = 0;
bool currentALTEnabled = false;
inputBitmap_t currentState = 0ULL;
inputBitmap_t currentLow = 0ULL;
inputBitmap_t currentHigh = 0ULL;
clutchValue_t currentClutch = CLUTCH_NONE_VALUE;
clutchValue_t currentLeftAxis = CLUTCH_NONE_VALUE;
clutchValue_t currentRightAxis = CLUTCH_NONE_VALUE;

void hidImplementation::reset()
{
    currentPOV = 0;
    currentALTEnabled = false;
    currentState = 0;
}

void hidImplementation::reportInput(
    inputBitmap_t inputsLow,
    inputBitmap_t inputsHigh,
    uint8_t POVstate,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis,
    clutchValue_t clutchAxis)
{
    currentLow = inputsLow;
    currentHigh = inputsHigh;
    currentPOV = POVstate;
    currentClutch = clutchAxis;
    currentLeftAxis = leftAxis;
    currentRightAxis = rightAxis;
    currentALTEnabled = (inputsLow == 0ULL) && (inputsHigh != 0ULL);
    currentState = currentALTEnabled ? inputsHigh : inputsLow;
    // Serial.println(".");
}

void notify::bitePoint()
{
}

void hidImplementation::reportChangeInConfig()
{
}

void inputs::recalibrateAxes()
{
}

void batteryCalibration::restartAutoCalibration()
{
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

template <typename T>
void assertEquals(const char *text, T expected, T found)
{
    if (expected != found)
    {
        if (sizeof(T) <= 4)
            Serial.printf("[assertEquals] (%s). Expected: %d, found: %d\n", text, expected, found);
        else
            Serial.printf("[assertEquals] (%s). Expected: %lld, found: %lld\n", text, expected, found);
    }
}

template <typename T>
void assertAlmostEquals(const char *text, T expected, T found, T tolerance)
{
    T lowerLimit = expected - tolerance;
    T upperLimit = expected + tolerance;
    if ((found < lowerLimit) || (found > upperLimit))
    {
        if (sizeof(T) <= 4)
            Serial.printf("[assertAlmostEquals] (%s). Expected: %d, found: %d\n", text, expected, found);
        else
            Serial.printf("[assertAlmostEquals] (%s). Expected: %lld, found: %lld\n", text, expected, found);
    }
}

template <typename T>
void pushAssertEqualsRelease(inputBitmap_t bmp, const char *text, T expected, T *found)
{
    input.pushSeveral(bmp);
    assertEquals<T>(text, expected, *found);
    input.release();
}

void inputs::update()
{
}

//------------------------------------------------------------------
// Test groups for easier maintenance
//------------------------------------------------------------------

void TG_altEngagement()
{
    // Send input for ALT engagement: clutch paddles in ALT mode and ALT buttons in ALT mode,
    // test that ALT is engaged
    input.release();
    userSettings::setCPWorkingMode(CF_ALT);
    userSettings::setALTButtonsWorkingMode(true);
    input.hasAnalogAxes = true;
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);

    // With ALT button
    input.push(ALT);
    assertEquals<inputBitmap_t>("ALT, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("ALT, inputs state high", 0ULL, currentHigh);
    input.push(OTHER);
    assertEquals<inputBitmap_t>("ALT+OTHER, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("ALT+OTHER, inputs state high", BITMAP(OTHER), currentHigh);
    input.release();
    assertEquals<inputBitmap_t>("ALT release, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("ALT release, inputs state high", 0ULL, currentHigh);

    // With analog clutch paddles
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<inputBitmap_t>("Analog paddle, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("Analog paddle, inputs state high", 0ULL, currentHigh);
    assertEquals<clutchValue_t>("Analog paddle, left axis", CLUTCH_NONE_VALUE, currentLeftAxis);
    input.push(OTHER);
    assertEquals<inputBitmap_t>("Analog paddle+OTHER, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("Analog paddle+OTHER, inputs state high", BITMAP(OTHER), currentHigh);
    input.release();
    assertEquals<inputBitmap_t>("Analog release, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("Analog release, inputs state high", 0ULL, currentHigh);

    // With digital clutch paddles
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.hasAnalogAxes = false;
    input.push(LCLUTCH);
    assertEquals<inputBitmap_t>("Digital paddle, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("Digital paddle, inputs state high", 0ULL, currentHigh);
    input.push(OTHER);
    assertEquals<inputBitmap_t>("Digital paddle+OTHER, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("Digital paddle+OTHER, inputs state high", BITMAP(OTHER), currentHigh);
    input.release();
    assertEquals<inputBitmap_t>("Digital release, inputs state low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("Digital release, inputs state high", 0ULL, currentHigh);
    input.hasAnalogAxes = true;
}

void TG_altInButtonsMode()
{
    // Simulate ALT button usage while in "regular buttons" mode
    userSettings::setCPWorkingMode(CF_CLUTCH);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    userSettings::setALTButtonsWorkingMode(false);
    input.push(ALT);
    assertEquals<inputBitmap_t>("ALT push, buttons mode, low", ALT_B, currentLow);
    assertEquals<inputBitmap_t>("ALT push, buttons mode, high", 0ULL, currentHigh);
    input.push(OTHER);
    assertEquals<inputBitmap_t>("ALT+OTHER, buttons mode, low", ALT_B | BITMAP(OTHER), currentLow);
    assertEquals<inputBitmap_t>("ALT+OTHER, buttons mode, high", 0ULL, currentHigh);
    input.release();
    assertEquals<inputBitmap_t>("release, buttons mode, low", 0ULL, currentLow);
    assertEquals<inputBitmap_t>("release, buttons mode, high", 0ULL, currentHigh);
}

void TG_POV_validInput()
{
    // Simulate a single push of each DPAD direction,
    // test POV is detected,
    // test their input numbers are NOT detected.
    userSettings::setDPADWorkingMode(true);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    pushAssertEqualsRelease<uint8_t>(UP_B, "UP_B", 1, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(DOWN_B, "DOWN_B", 5, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(LEFT_B, "LEFT_B", 7, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(RIGHT_B, "RIGHT_B", 3, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(UP_B | LEFT_B, "UP_B | LEFT_B", 8, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(DOWN_B | LEFT_B, "DOWN_B | LEFT_B", 6, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(UP_B | RIGHT_B, "UP_B | RIGHT_B", 2, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(DOWN_B | RIGHT_B, "DOWN_B | RIGHT_B", 4, &currentPOV);
    input.push(UP);
    assertEquals<inputBitmap_t>("state at push", 0ULL, currentState);
    input.release(UP);
    assertEquals<inputBitmap_t>("state at release", 0ULL, currentState);
}

void TG_POV_invalidInput()
{
    // Simulate impossible DPAD input,
    // test POV is NOT detected,
    // test their input numbers are NOT detected.
    userSettings::setDPADWorkingMode(true);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    pushAssertEqualsRelease<uint8_t>(UP_B | DOWN_B, "UP_B | DOWN_B", 0, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(LEFT_B | RIGHT_B, "LEFT_B | RIGHT_B", 0, &currentPOV);
    input.pushSeveral(UP_B | DOWN_B);
    assertEquals<inputBitmap_t>("state at push", 0, currentState);
    input.release();
    assertEquals<inputBitmap_t>("state at release", 0, currentState);
}

void TG_POV_whileAlt()
{
    // Simulate DPAD operation while ALT is engaged,
    // test ALT button is not detected,
    // test POV is NOT detected,
    // test DPAD input numbers ARE detected
    userSettings::setDPADWorkingMode(true);
    userSettings::setALTButtonsWorkingMode(true);
    userSettings::setCPWorkingMode(CF_CLUTCH);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.push(ALT);
    assertEquals<inputBitmap_t>("ALT filtered state", 0, currentState);
    input.push(UP);
    assertEquals<uint8_t>("UP_B | ALT_B POV", 0, currentPOV);
    assertEquals<inputBitmap_t>("UP_B | ALT_B state at push", UP_B, currentState);
    assertEquals<bool>("UP_B | ALT_B altEnabled", true, currentALTEnabled);
    input.release(UP);
    assertEquals<inputBitmap_t>("UP_B | ALT_B state at release", 0, currentState);
    pushAssertEqualsRelease<uint8_t>(DOWN_B | ALT_B, "DOWN_B | ALT_B", 0, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(LEFT_B | ALT_B, "LEFT_B | ALT_B", 0, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(RIGHT_B | ALT_B, "RIGHT_B | ALT_B", 0, &currentPOV);
    input.release();
}

void TG_POV_ButtonsMode()
{
    // Simulate DPAD operation ,
    // test DPAD input numbers ARE detected,
    // test POV is not detected
    userSettings::setDPADWorkingMode(false);
    input.release();
    input.push(UP);
    assertEquals<inputBitmap_t>("UP, bitmap", UP_B, currentLow);
    assertEquals<uint8_t>("UP, pov", 0, currentPOV);
    input.push(RIGHT);
    assertEquals<inputBitmap_t>("UP+RIGHT, bitmap", UP_B | RIGHT_B, currentLow);
    assertEquals<uint8_t>("UP+RIGHT, pov", 0, currentPOV);
    input.release();
    assertEquals<inputBitmap_t>("release, bitmap", 0ULL, currentLow);
    assertEquals<uint8_t>("release, pov", 0, currentPOV);
    userSettings::setDPADWorkingMode(true);
}

void TG_cycleAlt()
{
    // Cycle working mode of ALT buttons
    userSettings::setCPWorkingMode(CF_CLUTCH);
    userSettings::setALTButtonsWorkingMode(true);
    pushAssertEqualsRelease<bool>(BMP_CYCLE_ALT, "Cycle alt 1", false, (bool *)&userSettings::altButtonsWorkingMode);
    pushAssertEqualsRelease<bool>(BMP_CYCLE_ALT, "Cycle alt 2", true, (bool *)&userSettings::altButtonsWorkingMode);
}

void TG_cycleClutchWorkingMode()
{
    // Cycle working mode of clutch paddles
    userSettings::setCPWorkingMode(CF_LAUNCH_CONTROL_MASTER_RIGHT);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 1", CF_CLUTCH, (clutchFunction_t *)&userSettings::cpWorkingMode);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 2", CF_AXIS, (clutchFunction_t *)&userSettings::cpWorkingMode);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 3", CF_ALT, (clutchFunction_t *)&userSettings::cpWorkingMode);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 4", CF_BUTTON, (clutchFunction_t *)&userSettings::cpWorkingMode);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 5", CF_LAUNCH_CONTROL_MASTER_LEFT, (clutchFunction_t *)&userSettings::cpWorkingMode);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 6", CF_LAUNCH_CONTROL_MASTER_RIGHT, (clutchFunction_t *)&userSettings::cpWorkingMode);
}

void TG_cycleDPADWorkingMode()
{
    // Cycle working mode of DPAD inputs
    userSettings::setDPADWorkingMode(true);
    input.push(CMD);
    pushAssertEqualsRelease<bool>(BMP_CYCLE_ALT, "Cycle dpad 1", true, (bool *)&userSettings::dpadWorkingMode);
    input.push(CYCLE_DPAD);
    pushAssertEqualsRelease<bool>(BMP_CYCLE_ALT, "Cycle dpad 2", false, (bool *)&userSettings::altButtonsWorkingMode);
    input.release();
    userSettings::setDPADWorkingMode(true);
}

void TG_selectClutchWorkingMode()
{
    // Test selection of specific clutch working modes
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_ALT_F, "CF_ALT", CF_ALT, (clutchFunction_t *)&userSettings::cpWorkingMode);
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_CLUTCH_F, "CF_CLUTCH", CF_CLUTCH, (clutchFunction_t *)&userSettings::cpWorkingMode);
    userSettings::setCPWorkingMode(CF_ALT);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_BUTTON_F, "CF_BUTTON", CF_BUTTON, (clutchFunction_t *)&userSettings::cpWorkingMode);
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_AXIS_F, "CF_AXIS", CF_AXIS, (clutchFunction_t *)&userSettings::cpWorkingMode);
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_LC_LEFT_F, "CF_LC_LEFT", CF_LAUNCH_CONTROL_MASTER_LEFT, (clutchFunction_t *)&userSettings::cpWorkingMode);
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_LC_RIGHT_F, "CF_LC_RIGHT", CF_LAUNCH_CONTROL_MASTER_RIGHT, (clutchFunction_t *)&userSettings::cpWorkingMode);
}

void TG_nonMappedCombinations()
{
    // Test that a button combination not mapped to a wheel command does
    // not trigger other wheel commands
    userSettings::setCPWorkingMode(CF_BUTTON);
    userSettings::setALTButtonsWorkingMode(true);
    input.pushSeveral(BMP_CYCLE_ALT | BMP_CYCLE_CLUTCH);
    assertEquals<clutchFunction_t>("CF_BUTTON", CF_BUTTON, userSettings::cpWorkingMode);
    assertEquals<bool>("alt mode", true, userSettings::altButtonsWorkingMode);
    input.release();
}

void TG_bitePointCalibration()
{
    // Simulate inputs for bite point calibration,
    // test that bite point changes as expected
    clutchValue_t biteP;
    userSettings::setCPWorkingMode(CF_CLUTCH);
    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);
    input.hasAnalogAxes = true;
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint <= biteP)
        Serial.printf("Invalid bite point. Expected > %d, Found: %d\n", biteP, userSettings::bitePoint);
    biteP = userSettings::bitePoint;
    input.push(DOWN);
    input.release(DOWN);
    input.push(DOWN);
    input.release(DOWN);
    if (userSettings::bitePoint >= biteP)
        Serial.printf("Invalid bite point. Expected < %d, Found: %d\n", biteP, userSettings::bitePoint);

    /// Test that the bite point calibration is not triggered without paddle operation
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint != biteP)
        Serial.printf("Invalid bite point. Expected no change since no clutch paddle is pressed");

    /// Test that the bite point calibration is not triggered when both paddles are pressed
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint != biteP)
        Serial.printf("Invalid bite point. Expected no change since both clutch paddles are pressed");
}

void TG_bitePointCalibrationInLaunchControl()
{
    // Simulate inputs for bite point calibration,
    // test that bite point changes as expected

    /// --- Right paddle is master
    clutchValue_t biteP;
    input.hasAnalogAxes = true;
    userSettings::setCPWorkingMode(CF_LAUNCH_CONTROL_MASTER_RIGHT);
    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint <= biteP)
        Serial.printf("Master=right. Invalid bite point. Expected > %d, Found: %d\n", biteP, userSettings::bitePoint);
    biteP = userSettings::bitePoint;
    input.push(DOWN);
    input.release(DOWN);
    input.push(DOWN);
    input.release(DOWN);
    if (userSettings::bitePoint >= biteP)
        Serial.printf("Master=right. Invalid bite point. Expected < %d, Found: %d\n", biteP, userSettings::bitePoint);

    //// Test that the master paddle does not trigger bite point calibration
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint != biteP)
        Serial.printf("Master=right. Invalid bite point. Expected no change");

    /// --- Left paddle is master
    userSettings::setCPWorkingMode(CF_LAUNCH_CONTROL_MASTER_LEFT);
    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint <= biteP)
        Serial.printf("Master=left. Invalid bite point. Expected > %d, Found: %d\n", biteP, userSettings::bitePoint);
    biteP = userSettings::bitePoint;
    input.push(DOWN);
    input.release(DOWN);
    input.push(DOWN);
    input.release(DOWN);
    if (userSettings::bitePoint >= biteP)
        Serial.printf("Master=left. Invalid bite point. Expected < %d, Found: %d\n", biteP, userSettings::bitePoint);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);

    //// Test that the master paddle does not trigger bite point calibration
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint != biteP)
        Serial.printf("Master=left. Invalid bite point. Expected no change");

    /// Test that the bite point calibration is not triggered without paddle operation
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint != biteP)
        Serial.printf("Master=left. Invalid bite point. Expected no change since no clutch paddle is pressed");

    /// Test that the bite point calibration is not triggered when both paddles are pressed
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    biteP = userSettings::bitePoint;
    input.push(UP);
    input.release(UP);
    if (userSettings::bitePoint != biteP)
        Serial.printf("Master=left. Invalid bite point. Expected no change since both clutch paddles are pressed");
}

void TG_dualClutch()
{
    // Simulate clutch paddle operation,
    // Test computed clutch position
    userSettings::setCPWorkingMode(CF_CLUTCH);
    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);
    input.hasAnalogAxes = true;
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("dual clutch: 0,0", currentClutch, (clutchValue_t)CLUTCH_NONE_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>("dual clutch: FULL,FULL", currentClutch, (clutchValue_t)CLUTCH_FULL_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    assertAlmostEquals<clutchValue_t>("dual clutch: 0,FULL", currentClutch, (clutchValue_t)userSettings::bitePoint, 1);
    userSettings::setBitePoint(CLUTCH_3_4_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assertAlmostEquals<clutchValue_t>("dual clutch: FULL,0", currentClutch, (clutchValue_t)userSettings::bitePoint, 1);
}

void TG_launchControl()
{
    // Simulate clutch paddle operation,
    // Test computed clutch position in launch control mode
    input.hasAnalogAxes = true;
    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);

    /// --- Left paddle is master
    userSettings::setCPWorkingMode(CF_LAUNCH_CONTROL_MASTER_LEFT);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("Launch control master left: 0,0", currentClutch, (clutchValue_t)CLUTCH_NONE_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>("Launch control master left: FULL,FULL", currentClutch, (clutchValue_t)CLUTCH_FULL_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>("Launch control master left: 0,FULL", currentClutch, (clutchValue_t)CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_3_4_VALUE, CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>("Launch control master left: 3/4,FULL", currentClutch, (clutchValue_t)CLUTCH_3_4_VALUE);
    input.axis(CLUTCH_3_4_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("Launch control master left: 3/4,0", currentClutch, (clutchValue_t)CLUTCH_3_4_VALUE);

    /// --- Right paddle is master
    userSettings::setCPWorkingMode(CF_LAUNCH_CONTROL_MASTER_RIGHT);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("Launch control master right: 0,0", currentClutch, (clutchValue_t)CLUTCH_NONE_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>("Launch control master right: FULL,FULL", currentClutch, (clutchValue_t)CLUTCH_FULL_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("Launch control master right: FULL,0", currentClutch, (clutchValue_t)CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_3_4_VALUE);
    assertEquals<clutchValue_t>("Launch control master right: FULL,3/4", currentClutch, (clutchValue_t)CLUTCH_3_4_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_3_4_VALUE);
    assertEquals<clutchValue_t>("Launch control master right: 0, 3/4", currentClutch, (clutchValue_t)CLUTCH_3_4_VALUE);
}

void TG_analogClutchInButtonsMode()
{
    // Simulate operation of analog clutch paddles in buttons mode,
    // test that analog axes are not detected
    // test that buttons are detected
    userSettings::setCPWorkingMode(CF_BUTTON);
    input.hasAnalogAxes = true;
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("(1) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(1) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(1) state", 0ULL, currentState);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("(2) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(2) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(2) state", BITMAP(LCLUTCH), currentState);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>("(3) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(3) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(3) state", BITMAP(LCLUTCH) | BITMAP(RCLUTCH), currentState);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>("(4) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(4) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(4) state", BITMAP(RCLUTCH), currentState);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("(5) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(5) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(5) state", 0ULL, currentState);
}

void TG_digitalClutchInAxisMode()
{
    // Simulate operation of digital clutch paddles in axis mode,
    // test that analog axes are detected
    // test that buttons are not detected
    userSettings::setCPWorkingMode(CF_AXIS);
    input.hasAnalogAxes = false;
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.release();
    assertEquals<clutchValue_t>("(1) left", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<clutchValue_t>("(1) right", (clutchValue_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assertEquals<inputBitmap_t>("(1) state", 0ULL, currentState);
    input.push(LCLUTCH);
    assertEquals<clutchValue_t>("(2) left", (clutchValue_t)CLUTCH_FULL_VALUE, currentLeftAxis);
    assertEquals<clutchValue_t>("(2) right", (clutchValue_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assertEquals<inputBitmap_t>("(2) state", 0ULL, currentState);
    input.push(RCLUTCH);
    assertEquals<clutchValue_t>("(3) left", (clutchValue_t)CLUTCH_FULL_VALUE, currentLeftAxis);
    assertEquals<clutchValue_t>("(3) right", (clutchValue_t)CLUTCH_FULL_VALUE, currentRightAxis);
    assertEquals<inputBitmap_t>("(3) state", 0ULL, currentState);
    input.release(LCLUTCH);
    assertEquals<clutchValue_t>("(3) left", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<clutchValue_t>("(3) right", (clutchValue_t)CLUTCH_FULL_VALUE, currentRightAxis);
    assertEquals<inputBitmap_t>("(3) state", 0ULL, currentState);
    input.release();
    assertEquals<clutchValue_t>("(4) left", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<clutchValue_t>("(4) right", (clutchValue_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assertEquals<inputBitmap_t>("(4) state", 0ULL, currentState);
    input.hasAnalogAxes = true;
}

void TG_digitalClutchInButtonsMode()
{
    // Simulate operation of digital clutch paddles in buttons mode,
    // test that analog axes are not detected
    // test that buttons are detected
    userSettings::setCPWorkingMode(CF_BUTTON);
    input.hasAnalogAxes = false;
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.release();
    assertEquals<clutchValue_t>("(1) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(1) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(1) state", 0ULL, currentState);
    input.push(LCLUTCH);
    assertEquals<clutchValue_t>("(2) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(2) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(2) state", BITMAP(LCLUTCH), currentState);
    input.push(RCLUTCH);
    assertEquals<clutchValue_t>("(3) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(3) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(3) state", BITMAP(RCLUTCH) | BITMAP(LCLUTCH), currentState);
    input.release(LCLUTCH);
    assertEquals<clutchValue_t>("(4) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(4) axis", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<inputBitmap_t>("(4) state", BITMAP(RCLUTCH), currentState);
    input.release();
}

void TG_analogClutchInAxisMode()
{
    // Simulate operation of analog clutch paddles in axis mode,
    // test that analog axes are detected,
    // test that combined clutch axis is not detected
    // test that buttons are not detected
    userSettings::setCPWorkingMode(CF_AXIS);
    input.hasAnalogAxes = true;
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>("(1) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(1) left", (clutchValue_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assertEquals<clutchValue_t>("(1) right", (clutchValue_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assertEquals<inputBitmap_t>("(1) state", 0ULL, currentState);
    input.axis(CLUTCH_1_4_VALUE, CLUTCH_3_4_VALUE);
    assertEquals<clutchValue_t>("(2) clutch", (clutchValue_t)CLUTCH_NONE_VALUE, currentClutch);
    assertEquals<clutchValue_t>("(2) left", (clutchValue_t)CLUTCH_1_4_VALUE, currentLeftAxis);
    assertEquals<clutchValue_t>("(2) right", (clutchValue_t)CLUTCH_3_4_VALUE, currentRightAxis);
    assertEquals<inputBitmap_t>("(2) state", 0ULL, currentState);
}

void TG_repeatedCommand()
{
    // Send input twice in a row without a real change in those inputs,
    // test that the corresponding command is not executed twice
    userSettings::setCPWorkingMode(CF_CLUTCH);
    userSettings::setALTButtonsWorkingMode(true);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.pushSeveral(BMP_CYCLE_ALT);
    assertEquals<bool>("Repeated input 1st", false, userSettings::altButtonsWorkingMode);
    input.repeat();
    assertEquals<bool>("Repeated input 2nd", false, userSettings::altButtonsWorkingMode);
    input.release();
}

void TG_userMappedInput()
{
    // Adjust user-defined button map
    userSettings::resetButtonsMap();
    userSettings::setButtonMap(OTHER, OTHER_MAP, OTHER_MAP_ALT);
    userSettings::setALTButtonsWorkingMode(true);
    assertEquals<inputNumber_t>("map 1", OTHER_MAP, userSettings::buttonsMap[0][OTHER]);
    assertEquals<inputNumber_t>("map 2", OTHER_MAP_ALT, userSettings::buttonsMap[1][OTHER]);

    // send input and check
    input.release();
    input.push(OTHER);
    assertEquals<inputBitmap_t>("OTHER->OTHER_MAP (low)", BMP_OTHER_MAP_LOW, currentLow);
    assertEquals<inputBitmap_t>("OTHER->OTHER_MAP (high)", BMP_OTHER_MAP_HIGH, currentHigh);
    input.release();
    input.push(ALT);
    input.push(OTHER);
    assertEquals<inputBitmap_t>("OTHER_ALT->OTHER_MAP_ALT (low)", BMP_OTHER_MAP_ALT_LOW, currentLow);
    assertEquals<inputBitmap_t>("OTHER_ALT->OTHER_MAP_ALT (high)", BMP_OTHER_MAP_ALT_HIGH, currentHigh);
    input.release();

    // Reset
    userSettings::resetButtonsMap();
    assertEquals<inputNumber_t>("reset map 1", UNSPECIFIED_INPUT_NUMBER, userSettings::buttonsMap[0][OTHER]);
    assertEquals<inputNumber_t>("reset map 2", UNSPECIFIED_INPUT_NUMBER, userSettings::buttonsMap[1][OTHER]);
}

void TG_securityLock()
{
    // initialize
    userSettings::setSecurityLock(false);
    assertEquals<bool>("lock 1-1", false, userSettings::securityLock);

    // send input and check
    input.release();
    input.push(CMD);
    assertEquals<bool>("lock 1-2", false, userSettings::securityLock);
    input.push(OTHER);
    assertEquals<bool>("lock 1-3", false, userSettings::securityLock);
    input.push(UP);
    assertEquals<bool>("lock 2", true, userSettings::securityLock);
    input.release();
    input.push(CMD);
    input.push(OTHER);
    input.push(UP);
    input.release();
    assertEquals<bool>("lock 2", false, userSettings::securityLock);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    // Initialization
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    inputHub::setALTInputNumbers({ALT});
    inputHub::setDPADControls(UP, DOWN, LEFT, RIGHT);
    inputHub::cycleALTButtonsWorkingMode_setInputNumbers(COMBINATION_CYCLE_ALT);
    inputHub::cycleCPWorkingMode_setInputNumbers(COMBINATION_CYCLE_CLUTCH);
    inputHub::cpWorkingMode_setInputNumbers(
        COMBINATION_SELECT_CLUTCH_F,
        COMBINATION_SELECT_AXIS_F,
        COMBINATION_SELECT_ALT_F,
        COMBINATION_SELECT_BUTTON_F,
        COMBINATION_SELECT_LC_LEFT_F,
        COMBINATION_SELECT_LC_RIGHT_F);
    inputHub::setClutchCalibrationInputNumbers(UP, DOWN);
    inputHub::setClutchInputNumbers(LCLUTCH, RCLUTCH);
    inputHub::cycleSecurityLock_setInputNumbers(COMBINATION_SECURITY_LOCK);

    // Start
    Serial.println("-- GO --");

    assertEquals<inputBitmap_t>("state at start", 0, currentState);

    Serial.println("- simulate ALT engagement with clutch paddles or buttons -");
    TG_altEngagement();

    Serial.println("- simulate ALT button input when working mode is regular button -");
    TG_altInButtonsMode();

    Serial.println("- simulate POV operation (valid input) -");
    TG_POV_validInput();

    Serial.println("- simulate POV operation (invalid input) -");
    TG_POV_invalidInput();

    Serial.println("- simulate POV operation while ALT pushed -");
    TG_POV_whileAlt();

    Serial.println("- simulate POV operation in buttons mode -");
    TG_POV_ButtonsMode();

    Serial.println("- simulate cycle ALT working mode -");
    TG_cycleAlt();

    Serial.println("- simulate cycle clutch working mode -");
    TG_cycleClutchWorkingMode();

    Serial.println("- simulate cycle DPAD working mode -");
    TG_cycleDPADWorkingMode();

    Serial.println("- simulate explicit selection of clutch working mode -");
    TG_selectClutchWorkingMode();

    Serial.println("- simulate non-mapped button combinations -");
    TG_nonMappedCombinations();

    Serial.println("- simulate bite point calibration -");
    TG_bitePointCalibration();

    Serial.println("- simulate bite point calibration in launch control mode -");
    TG_bitePointCalibrationInLaunchControl();

    Serial.println("- simulate dual clutch operation -");
    TG_dualClutch();

    Serial.println("- simulate analog clutch operation in buttons mode -");
    TG_analogClutchInButtonsMode();

    Serial.println("- simulate digital clutch operation in axis mode -");
    TG_digitalClutchInAxisMode();

    Serial.println("- simulate digital clutch operation in buttons mode -");
    TG_digitalClutchInButtonsMode();

    Serial.println("- simulate analog clutch operation in axis mode -");
    TG_analogClutchInAxisMode();

    Serial.println("- simulate repeated input without real change in inputs state -");
    TG_repeatedCommand();

    Serial.println("- simulate input in user-defined buttons map -");
    TG_userMappedInput();

    Serial.println("- simulate security lock -");
    TG_securityLock();

    Serial.println("- simulate launch control -");
    TG_launchControl();

    Serial.println("-- END --");
    for (;;)
        ;
}

void loop()
{
}