#include "sensors/battery_monitor.h"

BatteryMonitor::BatteryMonitor(uint8_t pin, float dividerRatio, float minVoltage, float maxVoltage)
    : _pin(pin), _dividerRatio(dividerRatio), _minVoltage(minVoltage), _maxVoltage(maxVoltage)
{
}

void BatteryMonitor::begin()
{
    pinMode(_pin, INPUT);
}

float BatteryMonitor::readVoltage()
{
    // Take multiple samples to smooth out ADC noise on ESP32
    uint32_t rawSum = 0;
    const int SAMPLES = 8;
    for (int i = 0; i < SAMPLES; i++) {
        rawSum += analogRead(_pin);
        delayMicroseconds(200);
    }
    float rawAvg = (float)rawSum / SAMPLES;

    // ESP32 12-bit ADC (0 - 4095) with nominal 3.3V reference
    float vadc = (rawAvg / 4095.0f) * 3.3f;
    float vbat = vadc * _dividerRatio;

    return vbat;
}

int BatteryMonitor::readPercentage()
{
    float vbat = readVoltage();
    if (vbat <= _minVoltage) return 0;
    if (vbat >= _maxVoltage) return 100;

    int pct = (int)(((vbat - _minVoltage) / (_maxVoltage - _minVoltage)) * 100.0f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

bool BatteryMonitor::isLowBattery()
{
    return readVoltage() <= (_minVoltage + 0.1f);
}
