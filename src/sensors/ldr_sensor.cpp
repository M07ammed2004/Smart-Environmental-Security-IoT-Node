#include "sensors/ldr_sensor.h"

LDRSensor::LDRSensor(uint8_t pin)
    : _pin(pin)
{
}

void LDRSensor::begin()
{
    pinMode(_pin, INPUT);
}

int LDRSensor::readRaw()
{
    return analogRead(_pin);
}

float LDRSensor::readNormalized()
{
    int raw = readRaw();
    float pct = ((float)raw / 4095.0f) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}
