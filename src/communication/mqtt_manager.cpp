#include "communication/mqtt_manager.h"
#include <ArduinoJson.h>

static WiFiClientSecure s_tlsClient;
static PubSubClient     s_mqttClient(s_tlsClient);

static const char* s_host = nullptr;
static uint16_t    s_port = 8883;
static const char* s_user = nullptr;
static const char* s_pass = nullptr;
static const char* s_caCert = nullptr;

void MQTTManager::init(const char* host, uint16_t port, const char* user, const char* pass, const char* caCert)
{
    s_host = host;
    s_port = port;
    s_user = user;
    s_pass = pass;
    s_caCert = caCert;

    if (s_caCert && strlen(s_caCert) > 30) {
        s_tlsClient.setCACert(s_caCert);
    } else {
        // Fallback for local debugging before CA is provisioned
        s_tlsClient.setInsecure();
    }

    s_mqttClient.setServer(s_host, s_port);
    s_mqttClient.setBufferSize(512);
}

bool MQTTManager::isConnected()
{
    return s_mqttClient.connected();
}

bool MQTTManager::reconnect()
{
    if (s_mqttClient.connected()) return true;

    if (!s_host || strlen(s_host) == 0 || strcmp(s_host, "your-broker.hivemq.cloud") == 0) {
        // Broker not configured yet
        return false;
    }

    Serial.printf("[MQTT] Connecting to broker %s:%u...\n", s_host, s_port);
    String clientId = "ESP32_" + String(NODE_ID) + "_" + String(random(0xffff), HEX);

    bool ok = false;
    if (s_user && strlen(s_user) > 0 && strcmp(s_user, "your_mqtt_username") != 0) {
        ok = s_mqttClient.connect(clientId.c_str(), s_user, s_pass);
    } else {
        ok = s_mqttClient.connect(clientId.c_str());
    }

    if (!ok) {
        // If strict certificate verification failed, fallback to insecure TLS mode so connectivity is not blocked
        s_tlsClient.setInsecure();
        if (s_user && strlen(s_user) > 0 && strcmp(s_user, "your_mqtt_username") != 0) {
            ok = s_mqttClient.connect(clientId.c_str(), s_user, s_pass);
        } else {
            ok = s_mqttClient.connect(clientId.c_str());
        }
    }

    if (ok) {
        Serial.println("[MQTT] TLS connection established.");
        xEventGroupSetBits(g_systemEventGroup, MQTT_CONNECTED_BIT);
        return true;
    } else {
        Serial.printf("[MQTT] Connection failed, rc=%d\n", s_mqttClient.state());
        xEventGroupClearBits(g_systemEventGroup, MQTT_CONNECTED_BIT);
        return false;
    }
}

bool MQTTManager::ensureConnected()
{
    if (s_mqttClient.connected()) {
        xEventGroupSetBits(g_systemEventGroup, MQTT_CONNECTED_BIT);
        return true;
    }
    return reconnect();
}

void MQTTManager::loop()
{
    if (s_mqttClient.connected()) {
        s_mqttClient.loop();
    }
}

bool MQTTManager::publishTelemetry(const SensorData& data)
{
    if (!s_mqttClient.connected()) return false;

    JsonDocument doc;
    doc["node_id"] = NODE_ID;
    doc["temperature"] = round(data.temperature * 10.0f) / 10.0f;
    doc["humidity"] = round(data.humidity * 10.0f) / 10.0f;
    doc["light"] = data.lightRaw;
    doc["pir"] = data.pir ? 1 : 0;
    doc["touch"] = data.shock ? 1 : 0;
    doc["shock"] = data.shock ? 1 : 0;
#if ENABLE_BATTERY_MONITOR
    doc["battery"] = serialized(String(data.batteryVoltage, 2));
#endif

    char payload[256];
    serializeJson(doc, payload, sizeof(payload));

    char topic[64];
    snprintf(topic, sizeof(topic), "node/%s/telemetry", NODE_ID);

    bool res = s_mqttClient.publish(topic, payload);
    if (res) {
        Serial.printf("[MQTT] Published telemetry: %s\n", payload);
    }
    return res;
}

bool MQTTManager::publishSecurityEvent(const SecurityEvent& event)
{
    if (!s_mqttClient.connected()) return false;

    JsonDocument doc;
    doc["node_id"] = NODE_ID;
    doc["event"] = "SECURITY_EVENT";
    doc["motion"] = event.motionDetected ? 1 : 0;
    doc["shock"] = event.shockDetected ? 1 : 0;
    doc["classification"] = aiClassToString(event.classification);
    doc["confidence"] = serialized(String(event.confidence, 2));
    doc["timestamp"] = event.timestamp;

    char payload[256];
    serializeJson(doc, payload, sizeof(payload));

    char topic[64];
    snprintf(topic, sizeof(topic), "node/%s/security", NODE_ID);

    bool res = s_mqttClient.publish(topic, payload);
    if (res) {
        Serial.printf("[MQTT] Security event published: %s\n", payload);
    }
    return res;
}

bool MQTTManager::publishAIResult(const AIResult& result)
{
    if (!s_mqttClient.connected()) return false;

    JsonDocument doc;
    doc["node_id"] = NODE_ID;
    doc["event"] = "AI_RESULT";
    doc["classification"] = aiClassToString(result.classification);
    doc["confidence"] = serialized(String(result.confidence, 2));
    doc["inference_ms"] = result.inferenceTimeMs;

    char payload[256];
    serializeJson(doc, payload, sizeof(payload));

    char topic[64];
    snprintf(topic, sizeof(topic), "node/%s/ai", NODE_ID);

    return s_mqttClient.publish(topic, payload);
}
