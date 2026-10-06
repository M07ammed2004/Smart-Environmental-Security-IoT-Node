#include "sensors/touch_sensor.h"

volatile bool TouchSensor::s_touchTriggered = false;
volatile uint32_t TouchSensor::s_lastTouchMs = 0;
static uint8_t s_touchPin = 26;

void IRAM_ATTR TouchSensor::isrHandler()
{
    s_touchTriggered = true;
    s_lastTouchMs = millis();
}

TouchSensor::TouchSensor(uint8_t pin, bool activeHigh)
    : _pin(pin), _activeHigh(activeHigh)
{
    s_touchPin = pin;
}

void TouchSensor::begin()
{
    pinMode(_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(_pin), isrHandler, RISING);
}

bool TouchSensor::isTouched()
{
    // Active if pin is currently HIGH or triggered within the last 500ms
    if (digitalRead(s_touchPin) == HIGH || s_touchTriggered || (millis() - s_lastTouchMs < 500)) {
        if (millis() - s_lastTouchMs >= 500 && digitalRead(s_touchPin) == LOW) {
            s_touchTriggered = false;
        }
        return true;
    }
    return false;
}
