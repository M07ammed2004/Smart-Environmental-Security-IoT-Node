#include "power/power_manager.h"
#include "pins.h"
#include "communication/wifi_manager.h"

void PowerManager::init()
{
    printWakeupReason();
}

void PowerManager::printWakeupReason()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("[POWER] Wakeup caused by external signal using RTC_IO (PIR motion detected!)");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("[POWER] Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("[POWER] Wakeup caused by periodic timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.println("[POWER] Wakeup caused by touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            Serial.println("[POWER] Wakeup caused by ULP program");
            break;
        default:
            Serial.printf("[POWER] Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
            break;
    }
}

void PowerManager::configureWakeSources(uint64_t sleepDurationSec)
{
    // Wakeup source 1: PIR Motion sensor on PIN_PIR (GPIO 27) HIGH
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_PIR, 1);

    // Wakeup source 2: Periodic timer
    if (sleepDurationSec > 0) {
        esp_sleep_enable_timer_wakeup(sleepDurationSec * 1000000ULL);
    }
}

void PowerManager::enterDeepSleep(uint64_t sleepDurationSec)
{
    Serial.println("[POWER] Preparing to enter Deep Sleep...");

    // Disconnect Wi-Fi to shut down radio
    WiFiManager::disconnect();

    // Configure wakeup sources
    configureWakeSources(sleepDurationSec);

    Serial.printf("[POWER] Entering Deep Sleep for %llu s (or until PIR motion)...\n", sleepDurationSec);
    Serial.flush();

    esp_deep_sleep_start();
}
