#pragma once

#include <Arduino.h>

class LDRSensor {
public:
    explicit LDRSensor(uint8_t pin);
    void begin();
    int readRaw();
    float readNormalized(); // 0.0 to 100.0%

private:
    uint8_t _pin;
};
