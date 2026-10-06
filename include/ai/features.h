#pragma once

#include "system_config.h"

// Number of input features
#define FEATURE_COUNT 5 // [temperature, humidity, lightRaw, pir, shock]

struct FeatureVector {
    float values[FEATURE_COUNT];
};

class FeatureExtractor {
public:
    static FeatureVector extract(const SensorData& data);
    static void normalize(FeatureVector& vec);
};
