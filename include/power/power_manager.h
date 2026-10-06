#pragma once

#include <Arduino.h>
#include <esp_sleep.h>

class PowerManager {
public:
    static void init();
    static void printWakeupReason();
    static void configureWakeSources(uint64_t sleepDurationSec = 60);
    static void enterDeepSleep(uint64_t sleepDurationSec = 60);
};
