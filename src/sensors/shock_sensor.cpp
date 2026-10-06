#include "sensors/shock_sensor.h"

volatile bool ShockSensor::s_shockTriggered = false;
volatile uint32_t ShockSensor::s_lastTriggerMs = 0;

void IRAM_ATTR ShockSensor::isrHandler()
{
    s_shockTriggered = true;
    s_lastTriggerMs = millis();
}

ShockSensor::ShockSensor(uint8_t pin, bool activeHigh)
    : _pin(pin), _activeHigh(activeHigh)
{
}

void ShockSensor::begin()
{
    pinMode(_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_pin), isrHandler, CHANGE);
}

bool ShockSensor::isShockDetected()
{
    // If triggered within the last 800ms, consider shock active
    if (s_shockTriggered || (millis() - s_lastTriggerMs < 800)) {
        if (millis() - s_lastTriggerMs >= 800) {
            s_shockTriggered = false;
        }
        return true;
    }
    return false;
}

