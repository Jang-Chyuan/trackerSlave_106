#include <unity.h>

#include "batteryStatus.h"

void test_empty_and_full_voltage_are_clamped()
{
    BatteryStatus status;

    const BatteryReading empty = status.read(3.0f);
    const BatteryReading full = status.read(4.3f);

    TEST_ASSERT_TRUE(empty.valid);
    TEST_ASSERT_EQUAL_UINT8(0, empty.percentage);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(BatteryLevel::EMPTY), static_cast<int>(empty.level));
    TEST_ASSERT_EQUAL_UINT8(100, full.percentage);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(BatteryLevel::FULL), static_cast<int>(full.level));
}

void test_voltage_is_converted_to_percentage()
{
    const BatteryReading reading = BatteryStatus().read(3.70f);

    TEST_ASSERT_TRUE(reading.valid);
    TEST_ASSERT_EQUAL_UINT8(50, reading.percentage);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(BatteryLevel::MEDIUM), static_cast<int>(reading.level));
}

void test_level_boundaries_are_reported()
{
    BatteryStatus status;

    TEST_ASSERT_EQUAL_INT(static_cast<int>(BatteryLevel::CRITICAL), static_cast<int>(status.read(3.40f).level));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(BatteryLevel::LOW_BATTERY), static_cast<int>(status.read(3.60f).level));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(BatteryLevel::HIGH_BATTERY), static_cast<int>(status.read(4.00f).level));
}

void test_invalid_voltage_configuration_is_rejected()
{
    const BatteryReading reading = BatteryStatus(4.20f, 3.20f).read(3.70f);

    TEST_ASSERT_FALSE(reading.valid);
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_and_full_voltage_are_clamped);
    RUN_TEST(test_voltage_is_converted_to_percentage);
    RUN_TEST(test_level_boundaries_are_reported);
    RUN_TEST(test_invalid_voltage_configuration_is_rejected);
    UNITY_END();
}

void loop()
{
}