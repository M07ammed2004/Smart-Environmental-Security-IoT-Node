#include <Arduino.h>
#include "system_config.h"
#include "power/power_manager.h"
#include "tasks/sensor_task.h"
#include "tasks/security_task.h"
#include "tasks/ai_task.h"
#include "tasks/mqtt_task.h"

// ==========================================
// Global FreeRTOS Handles Definition
// ==========================================
EventGroupHandle_t g_systemEventGroup   = nullptr;
QueueHandle_t      g_sensorQueue        = nullptr;
QueueHandle_t      g_securityEventQueue = nullptr;
QueueHandle_t      g_aiRequestQueue     = nullptr;
QueueHandle_t      g_aiResultQueue      = nullptr;
QueueHandle_t      g_mqttQueue          = nullptr;

void setup()
{
    // 1. Serial initialization
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("==================================================");
    Serial.println(" Smart Environmental & Security IoT Node (ESP32)  ");
    Serial.printf (" Firmware: %s | Node ID: %s\n", FIRMWARE_VERSION, NODE_ID);
    Serial.println("==================================================");

    // 2. Power Manager & Wake Reason Check
    PowerManager::init();

    // 3. ADC configuration
    analogReadResolution(ADC_RESOLUTION_BITS);

    // 4. Create Event Group
    g_systemEventGroup = xEventGroupCreate();
    if (g_systemEventGroup == nullptr) {
        Serial.println("[BOOT] Error creating system event group!");
    }

    // 5. Create FreeRTOS Queues
    g_sensorQueue = xQueueCreate(QUEUE_SENSOR_DATA_LEN, sizeof(SensorData));
    g_securityEventQueue = xQueueCreate(QUEUE_SECURITY_EVENT_LEN, sizeof(SecurityEvent));
    g_aiRequestQueue = xQueueCreate(QUEUE_AI_REQUEST_LEN, sizeof(SensorData));
    g_aiResultQueue = xQueueCreate(QUEUE_AI_RESULT_LEN, sizeof(AIResult));
    g_mqttQueue = xQueueCreate(QUEUE_MQTT_PUBLISH_LEN, sizeof(SensorData));

    if (!g_sensorQueue || !g_securityEventQueue || !g_aiRequestQueue || !g_aiResultQueue) {
        Serial.println("[BOOT] Error: Failed to allocate all FreeRTOS queues!");
    } else {
        Serial.println("[BOOT] FreeRTOS queues initialized successfully.");
    }

    // 6. Spawn FreeRTOS Tasks
    startSensorTask();
    startSecurityTask();
    startAITask();
    startMQTTTask();

    Serial.println("[BOOT] All tasks started. System running.");
}

void loop()
{
    // Minimal loop: FreeRTOS tasks manage all execution threads
    vTaskDelay(pdMS_TO_TICKS(1000));
}