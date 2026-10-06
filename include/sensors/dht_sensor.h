#pragma once

#include <Arduino.h>

class DHTSensor {
public:
    DHTSensor(uint8_t pin, uint8_t type);
    void begin();
    bool read(float &temperature, float &humidity);

private:
    uint8_t _pin;
    uint8_t _type;
    void* _dhtPtr; // Internal pointer to DHT object
    unsigned long _lastReadTime;
    float _cachedTemp;
    float _cachedHumidity;
};
