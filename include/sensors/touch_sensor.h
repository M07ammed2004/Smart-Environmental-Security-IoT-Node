#pragma once

#include <Arduino.h>

class TouchSensor {
public:
    explicit TouchSensor(uint8_t pin, bool activeHigh = true);
    void begin();
    static bool isTouched();

private:
    uint8_t _pin;
    bool _activeHigh;
    static void IRAM_ATTR isrHandler();
    static volatile bool s_touchTriggered;
    static volatile uint32_t s_lastTouchMs;
};
