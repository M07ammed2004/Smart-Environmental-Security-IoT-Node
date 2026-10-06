#pragma once

#include <Arduino.h>

class ShockSensor {
public:
    explicit ShockSensor(uint8_t pin, bool activeHigh = true);
    void begin();
    static bool isShockDetected();

private:
    uint8_t _pin;
    bool _activeHigh;
    static void IRAM_ATTR isrHandler();
    static volatile bool s_shockTriggered;
    static volatile uint32_t s_lastTriggerMs;
};
