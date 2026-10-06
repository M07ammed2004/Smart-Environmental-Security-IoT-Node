#pragma once

#include <Arduino.h>

class PIRSensor {
public:
    explicit PIRSensor(uint8_t pin);
    void begin();
    bool isMotionDetected();
    uint8_t getPin() const { return _pin; }

private:
    uint8_t _pin;
};
