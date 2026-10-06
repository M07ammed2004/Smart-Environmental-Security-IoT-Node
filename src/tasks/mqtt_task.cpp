#include "tasks/mqtt_task.h"
#include "system_config.h"
#include "communication/wifi_manager.h"
#include "communication/mqtt_manager.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

void startMQTTTask()
{
    xTaskCreatePinnedToCore(
        mqttTaskFunction,
        "MQTTTask",
        STACK_TASK_MQTT,
        nullptr,
        PRIORITY_TASK_MQTT,
        nullptr,
        0 // Core 0 (Protocol CPU, keeping Core 1 free for sensors/AI)
    );
}

void mqttTaskFunction(void* pvParameters)
{
    Serial.println("[MQTT] MQTT Task started on Core 0.");

    WiFiManager::init(WIFI_SSID, WIFI_PASSWORD);
    MQTTManager::init(MQTT_BROKER_HOST, MQTT_BROKER_PORT, MQTT_USERNAME, MQTT_PASSWORD, CA_CERT);

    TickType_t lastWifiAttempt = 0;
    TickType_t lastMqttAttempt = 0;
    TickType_t lastTelemetryPublish = 0;
    const TickType_t wifiRetryInterval = pdMS_TO_TICKS(10000); // 10s backoff
    const TickType_t mqttRetryInterval = pdMS_TO_TICKS(5000);  // 5s backoff
    const TickType_t telemetryInterval = pdMS_TO_TICKS(5000);  // 5s publish

    for (;;) {
        TickType_t now = xTaskGetTickCount();

        // 1. Maintain Wi-Fi
        if (!WiFiManager::isConnected()) {
            if (now - lastWifiAttempt > wifiRetryInterval) {
                lastWifiAttempt = now;
                WiFiManager::ensureConnected(12000);
            }
        }

        // 2. Maintain MQTT
        if (WiFiManager::isConnected()) {
            if (!MQTTManager::isConnected()) {
                if (now - lastMqttAttempt > mqttRetryInterval) {
                    lastMqttAttempt = now;
                    MQTTManager::ensureConnected();
                }
            } else {
                MQTTManager::loop();
            }
        }

        // 3. Process immediate Security Events (High priority)
        SecurityEvent secEvent;
        if (g_securityEventQueue != nullptr && xQueueReceive(g_securityEventQueue, &secEvent, 0) == pdPASS) {
            if (MQTTManager::isConnected()) {
                MQTTManager::publishSecurityEvent(secEvent);
            } else {
                Serial.println("[MQTT] Notice: Security event detected but MQTT is offline.");
            }
        }

        // 4. Process AI Results
        AIResult aiRes;
        if (g_aiResultQueue != nullptr && xQueueReceive(g_aiResultQueue, &aiRes, 0) == pdPASS) {
            if (MQTTManager::isConnected()) {
                MQTTManager::publishAIResult(aiRes);
            }
        }

        // 5. Periodic Telemetry publish (only consume queue if MQTT is connected)
        if (MQTTManager::isConnected() && (now - lastTelemetryPublish >= telemetryInterval)) {
            SensorData data;
            if (g_sensorQueue != nullptr && xQueueReceive(g_sensorQueue, &data, 0) == pdPASS) {
                lastTelemetryPublish = now;
                MQTTManager::publishTelemetry(data);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
