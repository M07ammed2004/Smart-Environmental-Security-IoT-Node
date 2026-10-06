#pragma once

#include <Arduino.h>

/**
 * @file config.h
 * @brief System configuration, intervals, task parameters and thresholds.
 */

// ==========================================
// Node Identification & Version
// ==========================================
#define NODE_ID                     "NODE_001"
#define FIRMWARE_VERSION            "1.0.0"

// ==========================================
// Sensor Hardware Settings
// ==========================================
// DHT Type: 11 for DHT11, 22 for DHT22
#define DHT_TYPE                    11 // 11 for DHT11 (or 22 for DHT22)

// ESP32 ADC Settings
#define ADC_RESOLUTION_BITS         12    // 0 - 4095
#define ADC_REF_VOLTAGE             3.3f  // ESP32 nominal ADC Vref

// ==========================================
// Battery Monitoring (Disabled as requested)
// ==========================================
#define ENABLE_BATTERY_MONITOR      0     // Set to 0 if not physically wired
#define BATTERY_DIVIDER_RATIO       2.0f
#define BATTERY_MIN_VOLTAGE         3.3f
#define BATTERY_MAX_VOLTAGE         4.2f

// ==========================================
// Sensor Sampling Intervals (ms)
// ==========================================
#define SENSOR_SAMPLE_INTERVAL_MS   2000  // DHT & LDR reading rate
#define BATTERY_SAMPLE_INTERVAL_MS  30000 // Battery read rate (30 seconds)

// ==========================================
// FreeRTOS Task Priorities (Higher number = Higher priority)
// ==========================================
#define PRIORITY_TASK_SENSOR        2
#define PRIORITY_TASK_SECURITY      4    // High priority for instant security reactions
#define PRIORITY_TASK_AI            3
#define PRIORITY_TASK_MQTT          2

// ==========================================
// FreeRTOS Task Stack Sizes (Bytes)
// ==========================================
#define STACK_TASK_SENSOR           4096
#define STACK_TASK_SECURITY         4096
#define STACK_TASK_AI               8192
#define STACK_TASK_MQTT             8192

// ==========================================
// Queue Sizes
// ==========================================
#define QUEUE_SENSOR_DATA_LEN       10
#define QUEUE_SECURITY_EVENT_LEN    5
#define QUEUE_AI_REQUEST_LEN        5
#define QUEUE_AI_RESULT_LEN         5
#define QUEUE_MQTT_PUBLISH_LEN      10

// ==========================================
// Power Management & Sleep Defaults
// ==========================================
#define DEFAULT_DEEP_SLEEP_SEC      60   // Periodic wake interval if no PIR motion
