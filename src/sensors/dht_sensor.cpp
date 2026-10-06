#include "sensors/dht_sensor.h"
#include <DHT.h>

DHTSensor::DHTSensor(uint8_t pin, uint8_t type)
    : _pin(pin), _type(type), _lastReadTime(0), _cachedTemp(0.0f), _cachedHumidity(0.0f)
{
    _dhtPtr = new DHT(_pin, _type);
}

void DHTSensor::begin()
{
    if (_dhtPtr) {
        static_cast<DHT*>(_dhtPtr)->begin();
    }
}

bool DHTSensor::read(float &temperature, float &humidity)
{
    if (!_dhtPtr) return false;

    DHT* dht = static_cast<DHT*>(_dhtPtr);
    float t = dht->readTemperature();
    float h = dht->readHumidity();

    if (isnan(t) || isnan(h)) {
        // In case of transient read error, return cached if available or error
        if (_lastReadTime > 0) {
            temperature = _cachedTemp;
            humidity = _cachedHumidity;
            return true;
        }
        return false;
    }

    _cachedTemp = t;
    _cachedHumidity = h;
    _lastReadTime = millis();
    temperature = t;
    humidity = h;
    return true;
}
