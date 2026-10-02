#include "IMUDriver.h"


// ============================================================
// Global object
// ============================================================

IMUDriver IMU;

namespace
{
uint8_t i2cScanAddresses[126]{};
uint8_t i2cScanCount = 0;
bool i2cScanComplete = false;
bool i2cScanFoundBmi270 = false;
}


// ============================================================
// begin()
// ============================================================

bool IMUDriver::begin()
{
    initialized = false;


    // External BMI270: power from 3V3, SDA=15, SCL=16.
    Wire.begin(
        IMU_SDA,
        IMU_SCL
    );

    Wire.setClock(400000);

    Serial.println("[IMU] Scanning I2C bus before sensor initialization...");
    i2cScanCount = 0;
    i2cScanFoundBmi270 = false;
    for (uint8_t address = 1; address < 127; ++address)
    {
        Wire.beginTransmission(address);
        const uint8_t error = Wire.endTransmission();
        if (error == 0)
        {
            i2cScanAddresses[i2cScanCount++] = address;
            Serial.printf("[IMU] I2C device found at 0x%02X\n", address);
            if (address == IMU_I2C_ADDR)
            {
                i2cScanFoundBmi270 = true;
            }
        }
    }
    i2cScanComplete = true;
    Serial.println(i2cScanFoundBmi270
        ? "[IMU] BMI270 address 0x68 detected on I2C bus"
        : "[IMU] BMI270 address 0x68 not detected on I2C bus");


    // --------------------------------------------------------
    // BMI270
    // --------------------------------------------------------

    int8_t err =
        imu.beginI2C(
            IMU_I2C_ADDR,
            Wire
        );


    if (err != BMI2_OK)
    {
        Serial.print("[IMU] BMI270 init failed, error = ");
        Serial.println(err);

        return false;
    }


    initialized = true;


    Serial.println("[IMU] BMI270 initialized");
    Serial.print("[IMU] SDA = ");
    Serial.println(IMU_SDA);

    Serial.print("[IMU] SCL = ");
    Serial.println(IMU_SCL);

    Serial.print("[IMU] Address = 0x");
    Serial.println(IMU_I2C_ADDR, HEX);


    return true;
}


// ============================================================
// read()
// ============================================================

bool IMUDriver::read(IMUSample& sample)
{
    // --------------------------------------------------------
    // Default invalid
    // --------------------------------------------------------

    sample = {};


    if (!initialized)
    {
        return false;
    }


    // --------------------------------------------------------
    // Read BMI270
    // --------------------------------------------------------

    // The library retains its previous data on failure. Do not timestamp it as new.
    const int8_t readError = imu.getSensorData();
    if (readError != BMI2_OK)
    {
        return false;
    }


    // --------------------------------------------------------
    // Timestamp
    // --------------------------------------------------------

    sample.timestamp = millis();


    // --------------------------------------------------------
    // Accelerometer
    //
    // SparkFun BMI270 library:
    // data.accelX/Y/Z
    // --------------------------------------------------------

    sample.ax = imu.data.accelX;
    sample.ay = imu.data.accelY;
    sample.az = imu.data.accelZ;


    // --------------------------------------------------------
    // Gyroscope
    // --------------------------------------------------------

    sample.gx = imu.data.gyroX;
    sample.gy = imu.data.gyroY;
    sample.gz = imu.data.gyroZ;


    sample.valid = true;


    return true;
}


// ============================================================
// isReady()
// ============================================================

bool IMUDriver::isReady() const
{
    return initialized;
}


void IMUDriver::printI2CScanResults() const
{
    if (!i2cScanComplete)
    {
        Serial.println("[LOOP] I2C scan has not run");
        return;
    }

    Serial.printf("[LOOP] Cached I2C scan: %u device(s)\n", i2cScanCount);
    for (uint8_t index = 0; index < i2cScanCount; ++index)
    {
        Serial.printf("[LOOP] I2C device found at 0x%02X\n",
                      i2cScanAddresses[index]);
    }
    Serial.println(i2cScanFoundBmi270
        ? "[LOOP] BMI270 address 0x68 detected in cached scan"
        : "[LOOP] BMI270 address 0x68 not detected in cached scan");
}


// ============================================================
// sensor()
// ============================================================

BMI270& IMUDriver::sensor()
{
    return imu;
}