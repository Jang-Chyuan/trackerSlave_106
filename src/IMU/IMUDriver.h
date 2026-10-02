#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"

// ============================================================
// Heltec Wireless Tracker V2 + external BMI270
// ============================================================

#include "TrackerConfig.h"
#define IMU_SDA TRACKER_IMU_SDA
#define IMU_SCL TRACKER_IMU_SCL

#define IMU_I2C_ADDR    0x68


// ============================================================
// IMU Sample
// ============================================================

struct IMUSample
{
    uint32_t timestamp;

    float ax;
    float ay;
    float az;

    float gx;
    float gy;
    float gz;

    bool valid;
};


// ============================================================
// IMU Driver
// ============================================================

class IMUDriver
{
public:

    bool begin();

    bool read(IMUSample& sample);

    bool isReady() const;

    void printI2CScanResults() const;

    BMI270& sensor();

private:

    BMI270 imu;

    bool initialized = false;
};


// ============================================================
// Global IMU Driver
// ============================================================

extern IMUDriver IMU;
