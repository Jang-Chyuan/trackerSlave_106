#pragma once

#include <cstdint>

enum class BatteryLevel
{
    EMPTY,
    CRITICAL,
    LOW_BATTERY,
    MEDIUM,
    HIGH_BATTERY,
    FULL
};

struct BatteryReading
{
    float voltage;
    uint8_t percentage;
    BatteryLevel level;
    bool valid;
};

class BatteryStatus
{
public:
    BatteryStatus(float emptyVoltage = 3.20f, float fullVoltage = 4.20f);

    BatteryReading read(float voltage) const;
    BatteryReading readFromAdc() const;

private:
    float emptyVoltage;
    float fullVoltage;
};