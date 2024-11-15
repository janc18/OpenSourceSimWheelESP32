/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-18
 * @brief Implementation of the `capabilities` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

volatile uint32_t capabilities::flags = 0;
volatile inputBitmap_t capabilities::availableInputs = 0ULL;

// ----------------------------------------------------------------------------
// Flags
// ----------------------------------------------------------------------------

void capabilities::setFlag(deviceCapability_t newFlag, bool setOrClear)
{
    if (setOrClear)
        capabilities::flags = capabilities::flags | (1 << newFlag);
    else
        capabilities::flags = capabilities::flags & ~(1 << newFlag);
}

bool capabilities::hasFlag(deviceCapability_t flag)
{
    return capabilities::flags & (1 << flag);
}

void capabilities::addInputNumber(inputNumber_t number)
{
    if (number <= MAX_INPUT_NUMBER)
        capabilities::availableInputs = capabilities::availableInputs | BITMAP(number);
}