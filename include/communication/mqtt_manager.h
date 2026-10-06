#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "system_config.h"

class MQTTManager {
public:
    static void init(const char* host, uint16_t port, const char* user, const char* pass, const char* caCert = nullptr);
    static bool ensureConnected();
    static void loop();
    static bool publishTelemetry(const SensorData& data);
    static bool publishSecurityEvent(const SecurityEvent& event);
    static bool publishAIResult(const AIResult& result);
    static bool isConnected();

private:
    static bool reconnect();
};
