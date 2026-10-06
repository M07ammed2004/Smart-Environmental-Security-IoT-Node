#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    static void init(const char* ssid, const char* password);
    static bool isConnected();
    static bool ensureConnected(uint32_t timeoutMs = 10000);
    static void disconnect();
};
