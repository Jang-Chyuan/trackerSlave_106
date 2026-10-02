#include "batteryStatus.h"

#ifdef ARDUINO
#include <Arduino.h>
#include "freertos/semphr.h"
#endif

namespace
{
BatteryReading invalidReading(float voltage)
{
    return {voltage, 0, BatteryLevel::EMPTY, false};
}

BatteryLevel levelForPercentage(uint8_t percentage)
{
    if (percentage == 0)
    {
        return BatteryLevel::EMPTY;
    }

    if (percentage <= 20)
    {
        return BatteryLevel::CRITICAL;
    }

    if (percentage <= 40)
    {
        return BatteryLevel::LOW_BATTERY;
    }

    if (percentage <= 70)
    {
        return BatteryLevel::MEDIUM;
    }

    if (percentage < 100)
    {
        return BatteryLevel::HIGH_BATTERY;
    }

    return BatteryLevel::FULL;
}
}

BatteryStatus::BatteryStatus(float emptyVoltage, float fullVoltage)
    : emptyVoltage(emptyVoltage), fullVoltage(fullVoltage)
{
}

BatteryReading BatteryStatus::read(float voltage) const
{
    if (fullVoltage <= emptyVoltage || voltage != voltage)
    {
        return invalidReading(voltage);
    }

    if (voltage <= emptyVoltage)
    {
        return {voltage, 0, BatteryLevel::EMPTY, true};
    }

    if (voltage >= fullVoltage)
    {
        return {voltage, 100, BatteryLevel::FULL, true};
    }

    const float ratio = (voltage - emptyVoltage) / (fullVoltage - emptyVoltage);
    const uint8_t percentage = static_cast<uint8_t>(ratio * 100.0f + 0.5f);

    return {voltage, percentage, levelForPercentage(percentage), true};
}

BatteryReading BatteryStatus::readFromAdc() const
{
#ifdef ARDUINO
    static SemaphoreHandle_t adcMutex = xSemaphoreCreateMutex();
    if (!adcMutex || xSemaphoreTake(adcMutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return invalidReading(0.0f);
    constexpr float BATTERY_DIVIDER_RATIO = 4.9f;
    analogSetPinAttenuation(BATTERY_PIN, ADC_2_5db);

    pinMode(ADC_CTRL_PIN, OUTPUT);
    digitalWrite(ADC_CTRL_PIN, HIGH);
    delay(10);

    const uint32_t adcMillivolts = analogReadMilliVolts(BATTERY_PIN);
    digitalWrite(ADC_CTRL_PIN, LOW);
    xSemaphoreGive(adcMutex);

    const float voltage =
        static_cast<float>(adcMillivolts) * BATTERY_DIVIDER_RATIO / 1000.0f;

    return read(voltage);
#else
    return invalidReading(0.0f);
#endif
}