#pragma once

#include <Arduino.h>

/**
 * @file pins.h
 * @brief Hardware pin mapping for ESP32 DOIT DevKit V1 Smart Environmental & Security IoT Node.
 * 
 * IMPORTANT ADC NOTES:
 * - GPIO 34 and GPIO 35 are INPUT ONLY pins (no internal pull-up/pull-down).
 * - They belong to ADC1 (safe to read while Wi-Fi is active).
 * - Never apply > 3.3V to any GPIO. Battery monitoring MUST use an appropriate voltage divider.
 */

// ==========================================
// Security & Motion Sensors
// ==========================================
#define PIN_PIR             27   // Digital input with wake-up capability (RTC GPIO)
#define PIN_TOUCH           26   // Capacitive Touch sensor module (SIG / OUT)
#define PIN_SHOCK           PIN_TOUCH // Backward compatibility alias

// ==========================================
// Environmental Sensors
// ==========================================
#define PIN_DHT             4    // Single-bus digital for DHT11 / DHT22
#define PIN_LDR             34   // ADC1 Channel 6 (Input-only)

// ==========================================
// Power & Battery Monitoring
// ==========================================
#define PIN_BATTERY_ADC     35   // ADC1 Channel 7 (Input-only via voltage divider)

// ==========================================
// Actuators & Local Indicators
// ==========================================
#define PIN_STATUS_LED      2    // Built-in or external indicator LED
#define PIN_BUZZER          25   // Local alert buzzer (GPIO/DAC output)
