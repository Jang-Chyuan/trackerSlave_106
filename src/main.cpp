#include "TrackerConfig.h"
#include "core/DeviceIdentity.h"
#include <Arduino.h>
#include "IMU/IMUDriver.h"
#include "core/SystemManager.h"
#include "tasks/health_check.h"
#include "batteryStatus.h"
#include "LoRa/LoRaDriver.h"

namespace
{
const char *batteryLevelName(BatteryLevel level)
{
  switch (level)
  {
  case BatteryLevel::EMPTY: return "EMPTY";
  case BatteryLevel::CRITICAL: return "CRITICAL";
  case BatteryLevel::LOW_BATTERY: return "LOW";
  case BatteryLevel::MEDIUM: return "MEDIUM";
  case BatteryLevel::HIGH_BATTERY: return "HIGH";
  case BatteryLevel::FULL: return "FULL";
  }

  return "UNKNOWN";
}
}

BatteryStatus batteryStatus;

void setup()
{
    Serial.begin(115200);
    delay(1500);
    Serial.printf("[TRACKER V2] stage=%d, slave=%u, master=%u\n",
                  TRACKER_STAGE, SLAVE_DEVICE_ID, EXPECTED_MASTER_DEVICE_ID);
    // Establish safe idle states before powering the GNSS/TFT rail.
    digitalWrite(7, LOW); pinMode(7, OUTPUT); // FEM power off until LoRa.begin
    digitalWrite(4, LOW); pinMode(4, OUTPUT); // FEM disabled
    digitalWrite(5, LOW); pinMode(5, OUTPUT);
    digitalWrite(TRACKER_TFT_BACKLIGHT, LOW);
    pinMode(TRACKER_TFT_BACKLIGHT, OUTPUT);
    pinMode(TRACKER_USB_PRESENT, INPUT);
    systemInit();
    // ⭐ 開機立即做 rollback 判定
    if (TRACKER_NETWORK_ENABLED) runHealthCheck();
}

void loop()
{
  // Service poll reception continuously; battery reporting keeps its 5 s cadence.
  if (TRACKER_LORA_ENABLED) LoRa.loop();
  static uint32_t lastI2CScan = 0;
  static uint32_t lastBatteryRead = millis() - 5000U;
  const uint32_t now = millis();

  if (static_cast<uint32_t>(now - lastI2CScan) >= 10000U)
  {
    lastI2CScan = now;
    if (TRACKER_IMU_ENABLED) IMU.printI2CScanResults();
  }

  if (static_cast<uint32_t>(now - lastBatteryRead) < 5000U)
  {
    delay(5);
    return;
  }
  lastBatteryRead = now;
  const BatteryReading reading = batteryStatus.readFromAdc();

  if (reading.valid)
  {
    Serial.printf(
      "[BATTERY] pin=%u voltage=%.3f V level=%u%% (%s)\n",
      BATTERY_PIN,
      reading.voltage,
      reading.percentage,
      batteryLevelName(reading.level));
  }
  else
  {
    Serial.println("[BATTERY] invalid reading");
  }

  delay(5);
}
