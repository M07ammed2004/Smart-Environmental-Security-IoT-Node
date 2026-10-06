#include "ai/inference.h"

bool TinyMLEngine::init()
{
    // Initialized engine (ready for TFLite Micro / Edge AI weights)
    return true;
}

AIResult TinyMLEngine::runInference(const FeatureVector& features)
{
    uint32_t startUs = micros();
    AIResult res;

    // Feature values:
    // [0] = temp_norm, [1] = hum_norm, [2] = light_norm, [3] = pir, [4] = shock
    float pir = features.values[3];
    float shock = features.values[4];
    float light = features.values[2];

    // Baseline heuristic classification until deployed TinyML model replaces it:
    // PIR + Shock or PIR in darkness -> SUSPICIOUS
    // PIR alone in normal light -> HUMAN_ACTIVITY
    // Otherwise -> NORMAL
    if (shock > 0.5f || (pir > 0.5f && light < 0.2f)) {
        res.classification = AI_SUSPICIOUS;
        res.confidence = 0.92f;
    } else if (pir > 0.5f) {
        res.classification = AI_HUMAN_ACTIVITY;
        res.confidence = 0.88f;
    } else {
        res.classification = AI_NORMAL;
        res.confidence = 0.99f;
    }

    res.inferenceTimeMs = (micros() - startUs) / 1000;
    if (res.inferenceTimeMs == 0) res.inferenceTimeMs = 1;

    return res;
}
