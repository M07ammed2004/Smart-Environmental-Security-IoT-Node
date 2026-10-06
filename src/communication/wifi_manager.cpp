#include "communication/wifi_manager.h"
#include "system_config.h"

static const char* s_ssid = nullptr;
static const char* s_pass = nullptr;

void WiFiManager::init(const char* ssid, const char* password)
{
    s_ssid = ssid;
    s_pass = password;
    WiFi.mode(WIFI_STA);
}

bool WiFiManager::isConnected()
{
    return (WiFi.status() == WL_CONNECTED);
}

bool WiFiManager::ensureConnected(uint32_t timeoutMs)
{
    if (isConnected()) {
        xEventGroupSetBits(g_systemEventGroup, WIFI_CONNECTED_BIT);
        return true;
    }

    if (!s_ssid || strlen(s_ssid) == 0 || strcmp(s_ssid, "YOUR_WIFI_SSID") == 0) {
        // Unconfigured dummy credentials
        xEventGroupClearBits(g_systemEventGroup, WIFI_CONNECTED_BIT);
        return false;
    }

    Serial.printf("[WIFI] Connecting to SSID: %s (timeout: %u s)...\n", s_ssid, timeoutMs / 1000);
    WiFi.begin(s_ssid, s_pass);

    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startMs < timeoutMs)) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WIFI] Connected! IP: %s (Signal: %d dBm)\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
        xEventGroupSetBits(g_systemEventGroup, WIFI_CONNECTED_BIT);

        // Sync NTP Time required for TLS X509 certificate date validation
        Serial.println("[TIME] Synchronizing NTP time for TLS...");
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
        time_t now = time(nullptr);
        uint32_t startWait = millis();
        while (now < 1700000000 && (millis() - startWait < 5000)) {
            delay(250);
            now = time(nullptr);
        }
        if (now >= 1700000000) {
            Serial.println("[TIME] Time synchronized successfully.");
        } else {
            Serial.println("[TIME] Notice: NTP sync still in progress in background.");
        }
        return true;
    } else {
        Serial.println("[WIFI] Connection failed or timed out. Will retry...");
        xEventGroupClearBits(g_systemEventGroup, WIFI_CONNECTED_BIT);
        return false;
    }
}

void WiFiManager::disconnect()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    xEventGroupClearBits(g_systemEventGroup, WIFI_CONNECTED_BIT);
    Serial.println("[WIFI] Wi-Fi disconnected and radio powered down.");
}
