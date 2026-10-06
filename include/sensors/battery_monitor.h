#pragma once

#include <Arduino.h>

class BatteryMonitor {
public:
    BatteryMonitor(uint8_t pin, float dividerRatio, float minVoltage = 3.3f, float maxVoltage = 4.2f);
    void begin();
    float readVoltage();
    int readPercentage();
    bool isLowBattery();

private:
    uint8_t _pin;
    float _dividerRatio;
    float _minVoltage;
    float _maxVoltage;
};
