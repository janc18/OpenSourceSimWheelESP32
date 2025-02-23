/**
 * @file PolledInput.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Detect input events in a polling (or sampling) loop.
 *
 * @details Events are detected by a polling loop that runs in a separate thread.
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __POLLEDINPUT_H__
#define __POLLEDINPUT_H__

#include "SimWheelTypes.h"
#include "esp32-hal.h"
#include "driver/i2c.h"

/**
 * @brief Base class for all polled inputs
 *
 */
class PolledInput
{
protected:
    PolledInput *nextInChain;

public:
    /**
     * @brief Construct a new Polled Input object
     *
     * @param[in] nextInChain Another instance to build a chain, or nullptr
     */
    PolledInput(PolledInput *nextInChain = nullptr);

    /**
     * @brief Get the next PolledInput object in a chain
     *
     * @return PolledInput* Next object in the chain or nullptr if none
     */
    PolledInput *getNextInChain();

    /**
     * @brief Check if an instance is already inserted in a chain
     *
     * @param instance Any instance
     * @param firstInChain Pointer to the first item in the chain
     * @return true if `instance` is chained or null
     * @return false if `instance` is not chained
     */
    static bool contains(PolledInput *instance, PolledInput *firstInChain);
};

/**
 * @brief Base class for all polled switches
 *
 */
class DigitalPolledInput : public PolledInput
{
public:
    // for read only
    inputBitmap_t mask;

protected:
    /**
     * @brief Check and initialize a GPIO pin for digital output
     *
     * @param aPin GPIO pin
     */
    void checkAndInitializeSelectorPin(gpio_num_t aPin);

    /**
     * @brief Check and initialize a GPIO pin for digital input
     *
     * @param aPin GPIO pin
     * @param enablePullDown True to enable internal pull-down resistor
     * @param enablePullUp True to enable internal pull-up resistor
     */
    void checkAndInitializeInputPin(gpio_num_t aPin, bool enablePullDown = true, bool enablePullUp = false);

    /**
     * @brief Compute a mask for an array of input numbers.
     *
     * @param inputNumbersArray Array of input numbers.
     * @param inputsCount Number of items (input numbers) in the previous array.
     */
    static inputNumber_t computeMask(const inputNumber_t inputNumbersArray[], uint8_t inputsCount);

public:
    /**
     * @brief Construct a new Digital Polled Input object
     *
     * @param[in] nextInChain Another instance to build a chain, or nullptr
     */
    DigitalPolledInput(DigitalPolledInput *nextInChain = nullptr);

    /**
     * @brief Read the current state of the inputs (pressed or released)
     *
     * @param lastState State of the same inputs as recorded in the previous iteration.
     *                  Whether the current state is unknown, `lastState` should be returned.
     *                  Should be set to zero at first call.
     * @return inputBitmap_t Current state of the inputs (a bit set to 1 means a pressed button).
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) = 0;

    /**
     * @brief Read the combined state of all inputs in a chain
     *
     * @param lastState State of all inputs as recorded in the previous iteration.
     * @param firstInChain Pointer to the first item in the chain
     * @return inputBitmap_t Current state of all inputs
     */
    static inputBitmap_t readInChain(inputBitmap_t lastState, DigitalPolledInput *firstInChain);

    /**
     * @brief Get the combined mask of all instances in a chain
     *
     * @param firstInChain Pointer to the first item in the chain
     * @return inputBitmap_t Combined mask
     */
    static inputBitmap_t getChainMask(DigitalPolledInput *firstInChain);

    /**
     * @brief Abort execution if the given input number is not valid
     *
     * @param number Input number
     */
    static void abortOnInvalidInputNumber(inputNumber_t number);
};

/**
 * @brief Class for buttons attached to a single digital pin
 *
 */
class DigitalButton : public DigitalPolledInput
{
protected:
    gpio_num_t pinNumber;
    bool pullupOrPulldown;
    inputBitmap_t bitmap;

public:
    /**
     * @brief Construct a new Digital Button object
     *
     * @param[in] pinNumber GPIO pin where the button is attached to
     * @param[in] buttonNumber Assigned number for this button
     * @param[in] pullupOrPulldown TRUE if a pullup resistor is used (LOW signal when pressed).
     *                             FALSE if a pulldown resistor is used (HIGH signal when pressed).
     * @param[in] enableInternalPull TRUE if the internal pullup or pulldown resistor must be enabled.
     *                               Ignored if the GPIO pin does not provide a pull resistor.
     * @param[in] nextInChain Another instance to build a chain or nullptr.
     */
    DigitalButton(
        gpio_num_t pinNumber,
        inputNumber_t buttonNumber = UNSPECIFIED_INPUT_NUMBER,
        bool pullupOrPulldown = true,
        bool enableInternalPull = true,
        DigitalPolledInput *nextInChain = nullptr);

    virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

/**
 * @brief Class for all polled analog inputs (axis)
 *
 */
class AnalogAxisInput
{
protected:
    gpio_num_t pinNumber;
    int minADCReading;
    int maxADCReading;
    int lastADCReading;
    clutchValue_t lastValue;

public:
    /**
     * @brief When true, lowest voltage means highest axis value.
     *        When false, lowest voltage means lowest axis value.
     *
     */
    bool reversed;

    /**
     * @brief Construct a new Analog Axis Input object
     *
     * @param pinNumber ADC-capable pin number
     * @param reversed True if the highest voltage is for the idle position. False, otherwise.
     */
    AnalogAxisInput(
        gpio_num_t pinNumber,
        bool reversed = true);

    /**
     * @brief Get autocalibration data. Used for persistent storage.
     *
     * @param[out] minReading Minimum adc reading
     * @param[out] maxReading Maximum adc reading
     */
    void getCalibrationData(int *minReading, int *maxReading);

    /**
     * @brief Read current axis position. The axis must go from one
     *        end to the other for autocalibration.
     *
     * @param[out] value Current axis position.
     * @param[out] autocalibrated True if this axis has been autocalibrated.
     */
    void read(
        clutchValue_t &value,
        bool &autocalibrated);

    /**
     * @brief Force autocalibration.
     *
     */
    void resetCalibrationData();

    /**
     * @brief Set autocalibration data (loaded from persistent storage).
     *
     * @param[out] minReading Minimum adc reading
     * @param[out] maxReading Maximum adc reading
     */
    void setCalibrationData(int minReading, int maxReading);
};

/**
 * @brief Base class for I2C GPIO Expanders
 *
 */
class I2CInput : public DigitalPolledInput
{
protected:
    /// @brief I2C address in 8 bits format (no need to shift left),
    /// ready to overlap the R/W bit. By default, the R/W bit is set to "write".
    uint8_t deviceAddress;
    i2c_port_t busDriver;

public:
    /**
     * @brief Construct a new I2CInput object
     *
     * @param[in] address7bits Address of a slave device.
     * @param[in] useSecondaryBus TRUE if connected to the secondary bus, FALSE if connected to the primary bus.
     * @param[in] max_speed_mult Bus speed multiplier in the range from 1 to 4.
     * @param[in] nextInChain Another instance to build a chain, or nullptr.
     */
    I2CInput(
        uint8_t address7bits,
        bool useSecondaryBus = false,
        uint8_t max_speed_mult = 1,
        DigitalPolledInput *nextInChain = nullptr);
};

#define MAX_EXPANDED_GPIO_COUNT 16

/**
 * @brief Base class for buttons attached to an I2C GPIO expander
 * @note All buttons must work in negative logic (pulled up)
 *
 */
class I2CButtonsInput : public I2CInput
{
protected:
    uint8_t gpioCount;
    inputBitmap_t gpioBitmap[MAX_EXPANDED_GPIO_COUNT];

protected:
    /**
     * @brief Read the state of all GPIO pins in the expander
     *
     * @param state State of all GPIO pins in positive logic. A bit set to 1 means a closed switch.
     * @return true On success
     * @return false On failure
     */
    virtual bool getGPIOstate(inputBitmap_t &state) = 0;

public:
    /**
     * @brief Construct a new I2CButtonsInput object
     *
     * @param buttonsCount Count of attached buttons. Must not exceed the maximum allowed by the GPIO expander.
     * @param address7Bits I2C address in 7 bits format.
     * @param useSecondaryBus TRUE to use the secondary bus, FALSE to use the primary bus.
     * @param[in] max_speed_mult Bus speed multiplier in the range from 1 to 4.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    I2CButtonsInput(
        uint8_t buttonsCount,
        uint8_t address7Bits,
        bool useSecondaryBus = false,
        uint8_t max_speed_mult = 1,
        DigitalPolledInput *nextInChain = nullptr);

    virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

#endif
