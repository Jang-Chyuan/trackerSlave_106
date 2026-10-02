#pragma once
#ifndef TRACKER_STAGE
#define TRACKER_STAGE 4
#endif
#if TRACKER_STAGE < 1 || TRACKER_STAGE > 4
#error TRACKER_STAGE must be 1 through 4
#endif
#define TRACKER_LORA_ENABLED (TRACKER_STAGE >= 2)
#define TRACKER_TFT_ENABLED (TRACKER_STAGE >= 3)
#define TRACKER_IMU_ENABLED (TRACKER_STAGE >= 4)
#ifndef TRACKER_NETWORK_ENABLED
#define TRACKER_NETWORK_ENABLED (TRACKER_STAGE >= 4)
#endif
constexpr int TRACKER_VEXT = 3; // HIGH enables the shared GNSS/TFT rail
constexpr int TRACKER_GPS_RX = 33; // ESP32 RX <- UC6580 TX
constexpr int TRACKER_GPS_TX = 34;
constexpr int TRACKER_GPS_RESET = 35;
constexpr int TRACKER_TFT_BACKLIGHT = 21; // HIGH = illuminated
constexpr int TRACKER_IMU_SDA = 15;
constexpr int TRACKER_IMU_SCL = 16;
constexpr int TRACKER_USB_PRESENT = 47;
