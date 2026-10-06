#include "tasks/security_task.h"
#include "system_config.h"
#include "sensors/touch_sensor.h"
#include "sensors/shock_sensor.h"

void startSecurityTask()
{
    xTaskCreatePinnedToCore(
        securityTaskFunction,
        "SecurityTask",
        STACK_TASK_SECURITY,
        nullptr,
        PRIORITY_TASK_SECURITY,
        nullptr,
        1 // Core 1
    );
}

static void playAlarmSound(int beeps, int durationMs)
{
    for (int i = 0; i < beeps; i++) {
        // tone() generates a 2500 Hz square wave (compatible with passive modules and active buzzers)
        tone(PIN_BUZZER, 2500);
        digitalWrite(PIN_STATUS_LED, HIGH);
        vTaskDelay(pdMS_TO_TICKS(durationMs));
        noTone(PIN_BUZZER);
        digitalWrite(PIN_BUZZER, LOW);
        digitalWrite(PIN_STATUS_LED, LOW);
        if (i < beeps - 1) {
            vTaskDelay(pdMS_TO_TICKS(80));
        }
    }
}

void securityTaskFunction(void* pvParameters)
{
    Serial.println("[SECURITY] Security Task started.");

    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_STATUS_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_STATUS_LED, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    bool lastPir = false;
    bool lastTouch = false;

    for (;;) {
        bool currentPir = (digitalRead(PIN_PIR) == HIGH);
        bool currentTouch = TouchSensor::isTouched() || ShockSensor::isShockDetected();

        // Detect rising edge or active security events
        if (currentPir != lastPir || currentTouch != lastTouch) {
            if (currentPir) {
                xEventGroupSetBits(g_systemEventGroup, MOTION_DETECTED_BIT);
                Serial.println("[SECURITY] Motion Detected (PIR HIGH)! Triggering Buzzer Alarm...");
            } else {
                xEventGroupClearBits(g_systemEventGroup, MOTION_DETECTED_BIT);
            }

            if (currentTouch) {
                xEventGroupSetBits(g_systemEventGroup, SHOCK_DETECTED_BIT);
                Serial.println("[SECURITY] Touch / Tamper Detected (GPIO 26)!");
            } else {
                xEventGroupClearBits(g_systemEventGroup, SHOCK_DETECTED_BIT);
            }

            // If an active event triggered
            if (currentPir || currentTouch) {
                xEventGroupSetBits(g_systemEventGroup, SECURITY_EVENT_BIT);

                // Sound the buzzer alarm: 3 rapid beeps for motion, 1 long beep for touch
                if (currentPir) {
                    playAlarmSound(3, 100);
                } else if (currentTouch) {
                    playAlarmSound(1, 250);
                }

                // Build security event
                SecurityEvent secEvent;
                secEvent.motionDetected = currentPir;
                secEvent.shockDetected = currentTouch;
                secEvent.classification = AI_NORMAL; // Will be updated by AI task or baseline
                secEvent.confidence = 1.0f;
                secEvent.timestamp = millis();

                // Send to AI Request Queue for inference
                if (g_aiRequestQueue != nullptr) {
                    SensorData latestContext;
                    // Peek latest telemetry context if available
                    if (g_sensorQueue != nullptr && xQueuePeek(g_sensorQueue, &latestContext, 0) == pdPASS) {
                        latestContext.pir = currentPir;
                        latestContext.shock = currentTouch;
                        xQueueSend(g_aiRequestQueue, &latestContext, 0);
                    } else {
                        // Fallback context
                        SensorData minimalContext = {};
                        minimalContext.pir = currentPir;
                        minimalContext.shock = currentTouch;
                        minimalContext.timestamp = millis();
                        xQueueSend(g_aiRequestQueue, &minimalContext, 0);
                    }
                }

                // Push to Security Event Queue for MQTT publishing
                if (g_securityEventQueue != nullptr) {
                    xQueueSend(g_securityEventQueue, &secEvent, 0);
                }

                vTaskDelay(pdMS_TO_TICKS(100));
                digitalWrite(PIN_STATUS_LED, LOW);
            } else {
                xEventGroupClearBits(g_systemEventGroup, SECURITY_EVENT_BIT);
            }

            lastPir = currentPir;
            lastTouch = currentTouch;
        }

        // Poll security pins at 50ms intervals
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
