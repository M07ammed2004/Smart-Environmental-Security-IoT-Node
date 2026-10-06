#include "ai/features.h"

FeatureVector FeatureExtractor::extract(const SensorData& data)
{
    FeatureVector vec;
    vec.values[0] = data.temperature;
    vec.values[1] = data.humidity;
    vec.values[2] = (float)data.lightRaw;
    vec.values[3] = data.pir ? 1.0f : 0.0f;
    vec.values[4] = data.shock ? 1.0f : 0.0f;
    return vec;
}

void FeatureExtractor::normalize(FeatureVector& vec)
{
    // Typical ranges:
    // temp: 0..50 C
    // hum:  0..100 %
    // light: 0..4095
    // pir: 0 or 1
    // shock: 0 or 1
    vec.values[0] = (vec.values[0] - 0.0f) / 50.0f;
    vec.values[1] = (vec.values[1] - 0.0f) / 100.0f;
    vec.values[2] = (vec.values[2] - 0.0f) / 4095.0f;
    // pir and shock are already 0 or 1
}
