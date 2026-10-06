#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include "config.h"
#include "pins.h"

/**
 * @file system_config.h
 * @brief Common data structures, EventGroup bits, and Queue Handles.
 */

// ==========================================
// AI Classification Enums & Structs
// ==========================================
enum AIClass {
    AI_NORMAL = 0,
    AI_HUMAN_ACTIVITY,
    AI_SUSPICIOUS
};

inline const char* aiClassToString(AIClass c) {
    switch (c) {
        case AI_NORMAL: return "NORMAL";
        case AI_HUMAN_ACTIVITY: return "HUMAN_ACTIVITY";
        case AI_SUSPICIOUS: return "SUSPICIOUS";
        default: return "UNKNOWN";
    }
}

struct AIResult {
    AIClass classification;
    float confidence;
    uint32_t inferenceTimeMs;
};

// ==========================================
// Sensor Telemetry Struct
// ==========================================
struct SensorData {
    float temperature;
    float humidity;
    int lightRaw;
    float batteryVoltage;
    int batteryPercent;

    bool pir;
    bool shock;

    uint32_t timestamp;
    bool valid;
};

// ==========================================
// Security Event Struct
// ==========================================
struct SecurityEvent {
    bool motionDetected;
    bool shockDetected;

    AIClass classification;
    float confidence;

    uint32_t timestamp;
};

// ==========================================
// EventGroup Bits (System-Level State)
// ==========================================
#define WIFI_CONNECTED_BIT      (1 << 0)
#define MQTT_CONNECTED_BIT      (1 << 1)
#define MOTION_DETECTED_BIT     (1 << 2)
#define SHOCK_DETECTED_BIT      (1 << 3)
#define SECURITY_EVENT_BIT      (1 << 4)
#define LOW_BATTERY_BIT         (1 << 5)
#define AI_READY_BIT            (1 << 6)

// ==========================================
// Global FreeRTOS Handles (Defined in main.cpp)
// ==========================================
extern EventGroupHandle_t g_systemEventGroup;
extern QueueHandle_t      g_sensorQueue;
extern QueueHandle_t      g_securityEventQueue;
extern QueueHandle_t      g_aiRequestQueue;
extern QueueHandle_t      g_aiResultQueue;
extern QueueHandle_t      g_mqttQueue;
