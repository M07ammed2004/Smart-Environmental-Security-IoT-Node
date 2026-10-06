#include "sensors/pir_sensor.h"

PIRSensor::PIRSensor(uint8_t pin)
    : _pin(pin)
{
}

void PIRSensor::begin()
{
    pinMode(_pin, INPUT);
}

bool PIRSensor::isMotionDetected()
{
    return digitalRead(_pin) == HIGH;
}
