#pragma once

#include "system_config.h"
#include "ai/features.h"

class TinyMLEngine {
public:
    static bool init();
    static AIResult runInference(const FeatureVector& features);
};
