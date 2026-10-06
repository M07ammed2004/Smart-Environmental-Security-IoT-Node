#include "tasks/sensor_task.h"
#include "system_config.h"
#include "sensors/dht_sensor.h"
#include "sensors/ldr_sensor.h"
#include "sensors/touch_sensor.h"
#include "sensors/shock_sensor.h"
#include "sensors/pir_sensor.h"
#include "sensors/battery_monitor.h"

static DHTSensor       s_dht(PIN_DHT, DHT_TYPE);
static LDRSensor       s_ldr(PIN_LDR);
static TouchSensor     s_touch(PIN_TOUCH);
static ShockSensor     s_shock(PIN_SHOCK);
static PIRSensor       s_pir(PIN_PIR);
static BatteryMonitor  s_battery(PIN_BATTERY_ADC, BATTERY_DIVIDER_RATIO, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE);

void startSensorTask()
{
    xTaskCreatePinnedToCore(
        sensorTaskFunction,
        "SensorTask",
        STACK_TASK_SENSOR,
        nullptr,
        PRIORITY_TASK_SENSOR,
        nullptr,
        1 // Core 1 (App CPU)
    );
}

void sensorTaskFunction(void* pvParameters)
{
    Serial.println("[SENSOR] Sensor Task started.");

    s_dht.begin();
    s_ldr.begin();
    s_touch.begin();
    s_shock.begin();
    s_pir.begin();
#if ENABLE_BATTERY_MONITOR
    s_battery.begin();
#endif

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(SENSOR_SAMPLE_INTERVAL_MS);

#if ENABLE_BATTERY_MONITOR
    uint32_t batteryCounter = 0;
    float currentVbat = 0.0f;
    int currentBatPct = 0;
#endif

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        SensorData data = {};
        data.timestamp = millis();
        data.valid = s_dht.read(data.temperature, data.humidity);
        data.lightRaw = s_ldr.readRaw();
        data.shock = TouchSensor::isTouched() || s_shock.isShockDetected();
        data.pir = s_pir.isMotionDetected();

#if ENABLE_BATTERY_MONITOR
        if (batteryCounter == 0 || (millis() - batteryCounter >= BATTERY_SAMPLE_INTERVAL_MS)) {
            batteryCounter = millis();
            currentVbat = s_battery.readVoltage();
            currentBatPct = s_battery.readPercentage();

            if (s_battery.isLowBattery()) {
                xEventGroupSetBits(g_systemEventGroup, LOW_BATTERY_BIT);
            } else {
                xEventGroupClearBits(g_systemEventGroup, LOW_BATTERY_BIT);
            }
        }
        data.batteryVoltage = currentVbat;
        data.batteryPercent = currentBatPct;
#else
        data.batteryVoltage = 0.0f;
        data.batteryPercent = 0;
#endif

        if (data.valid) {
#if ENABLE_BATTERY_MONITOR
            Serial.printf("[SENSOR] Temp: %.1f C, Humidity: %.1f %%, Light: %d, Touch: %d, Pir: %d, Vbat: %.2f V (%d%%)\n",
                          data.temperature, data.humidity, data.lightRaw, data.shock, data.pir, data.batteryVoltage, data.batteryPercent);
#else
            Serial.printf("[SENSOR] Temp: %.1f C, Humidity: %.1f %%, Light: %d, Touch: %d, Pir: %d\n",
                          data.temperature, data.humidity, data.lightRaw, data.shock, data.pir);
#endif
        } else {
            Serial.println("[SENSOR] Warning: DHT read failed, using cached values");
        }

        // Send telemetry data to sensorQueue (non-blocking overwrite or drop if full)
        if (g_sensorQueue != nullptr) {
            if (xQueueSend(g_sensorQueue, &data, (TickType_t)0) != pdPASS) {
                // Queue full: overwrite oldest item to keep freshest telemetry
                SensorData dummy;
                xQueueReceive(g_sensorQueue, &dummy, 0);
                xQueueSend(g_sensorQueue, &data, 0);
            }
        }
    }
}
