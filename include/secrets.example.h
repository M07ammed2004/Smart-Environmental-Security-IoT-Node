#pragma once

/**
 * @file secrets.example.h
 * @brief Template for secret credentials.
 * 
 * Instructions:
 * 1. Copy this file to `include/secrets.h`.
 * 2. Update with your actual Wi-Fi SSID, Password, and MQTT broker details.
 * 3. Never commit `secrets.h` to git (it is included in .gitignore).
 */

// ==========================================
// Wi-Fi Configuration
// ==========================================
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"

// ==========================================
// MQTT Broker Configuration
// ==========================================
#define MQTT_BROKER_HOST    "your-broker.hivemq.cloud"
#define MQTT_BROKER_PORT    8883 // TLS port
#define MQTT_USERNAME       "your_mqtt_username"
#define MQTT_PASSWORD       "your_mqtt_password"

// Root CA Certificate for TLS (PEM format)
static const char CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
... YOUR ROOT CA CERTIFICATE HERE ...
-----END CERTIFICATE-----
)EOF";
