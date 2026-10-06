#include "tasks/ai_task.h"
#include "system_config.h"
#include "ai/features.h"
#include "ai/inference.h"

void startAITask()
{
    xTaskCreatePinnedToCore(
        aiTaskFunction,
        "AITask",
        STACK_TASK_AI,
        nullptr,
        PRIORITY_TASK_AI,
        nullptr,
        1 // Core 1
    );
}

void aiTaskFunction(void* pvParameters)
{
    Serial.println("[AI] AI Inference Task started.");
    TinyMLEngine::init();
    xEventGroupSetBits(g_systemEventGroup, AI_READY_BIT);

    SensorData request;

    for (;;) {
        // Wait indefinitely for an AI request from Security or Sensor Task
        if (xQueueReceive(g_aiRequestQueue, &request, portMAX_DELAY) == pdPASS) {
            FeatureVector fv = FeatureExtractor::extract(request);
            FeatureExtractor::normalize(fv);

            AIResult res = TinyMLEngine::runInference(fv);

            Serial.printf("[AI] Classification: %s, Confidence: %.2f, Latency: %u ms\n",
                          aiClassToString(res.classification), res.confidence, res.inferenceTimeMs);

            // Put result into AI Result Queue
            if (g_aiResultQueue != nullptr) {
                xQueueSend(g_aiResultQueue, &res, 0);
            }
        }
    }
}
