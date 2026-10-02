
/*
功能：

GPS Task 每次 GPS 更新 (gps.location.isUpdated()) 產生新的 updateId
xQueueOverwrite() 保留最新 GPS
main.cpp 使用 xQueuePeek()
只有新 GPS 才判斷
移動距離 > 5m 才送
最短 LoRa 發送間隔 3 秒
60 秒 Heartbeat
適用 Heltec Wireless Tracker V2 + UC6580 + SX1262

*/

#include <Arduino.h>
#include "GpsTask.h"
#include "TrackerConfig.h"

QueueHandle_t gpsQueue = nullptr;
HardwareSerial GPSSerial(1);
TinyGPSPlus gps;
GPS_DATA gpsData;

static uint32_t gpsUpdateId = 0;

namespace
{
constexpr uint32_t GPS_AVERAGE_WINDOW_MS = 3000;
constexpr size_t GPS_AVERAGE_MAX_SAMPLES = 64;

struct GpsPositionSample
{
    int32_t lat;
    int32_t lon;
    uint32_t timestampMs;
};

GpsPositionSample positionSamples[GPS_AVERAGE_MAX_SAMPLES] = {};
size_t oldestSampleIndex = 0;
size_t positionSampleCount = 0;
int64_t latitudeSum = 0;
int64_t longitudeSum = 0;

void removeOldestPositionSample()
{
    latitudeSum -= positionSamples[oldestSampleIndex].lat;
    longitudeSum -= positionSamples[oldestSampleIndex].lon;
    oldestSampleIndex =
        (oldestSampleIndex + 1) % GPS_AVERAGE_MAX_SAMPLES;
    --positionSampleCount;
}

void addPositionSample(int32_t lat, int32_t lon, uint32_t timestampMs)
{
    while (positionSampleCount > 0 &&
           timestampMs - positionSamples[oldestSampleIndex].timestampMs >
               GPS_AVERAGE_WINDOW_MS)
    {
        removeOldestPositionSample();
    }

    if (positionSampleCount == GPS_AVERAGE_MAX_SAMPLES)
    {
        removeOldestPositionSample();
    }

    const size_t newSampleIndex =
        (oldestSampleIndex + positionSampleCount) % GPS_AVERAGE_MAX_SAMPLES;

    positionSamples[newSampleIndex] = {lat, lon, timestampMs};
    latitudeSum += lat;
    longitudeSum += lon;
    ++positionSampleCount;
}
} // namespace

//================================================
// GPS Power ON
//================================================

void gpsON()
{
    digitalWrite(TRACKER_VEXT, HIGH);
    pinMode(TRACKER_VEXT, OUTPUT);
    vTaskDelay(pdMS_TO_TICKS(300));
    digitalWrite(TRACKER_GPS_RESET, LOW);
    pinMode(TRACKER_GPS_RESET, OUTPUT);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(TRACKER_GPS_RESET, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

//================================================
// GPS Init
//================================================

void gpsInit()
{

    gpsON();

    GPSSerial.setRxBufferSize(2048);
    GPSSerial.begin(
        115200,
        SERIAL_8N1,
        TRACKER_GPS_RX,
        TRACKER_GPS_TX);

    Serial.println("[GPS] UART initialized: 115200 RX=33 TX=34; waiting for NMEA (not a fix)");
}

//================================================
// GPS Task
//================================================

void gpsTask(void *pvParameters)
{

    GPS_packet packet{};
    uint32_t lastDiagnosticMs = 0;

    while (1)
    {

        while (GPSSerial.available())
        {
            gps.encode(GPSSerial.read());
        }

        //----------------------------------------
        // New GPS location
        //----------------------------------------

        if (gps.location.isUpdated() && gps.location.isValid())
        {

            const int32_t latestLat =
                (int32_t)(gps.location.lat() * 1000000.0);

            const int32_t latestLon =
                (int32_t)(gps.location.lng() * 1000000.0);

            addPositionSample(
                latestLat,
                latestLon,
                millis());

            packet.updateId = ++gpsUpdateId;
            packet.capturedAtMs = millis();

            packet.lat =
                (int32_t)(latitudeSum / (int64_t)positionSampleCount);

            packet.lon =
                (int32_t)(longitudeSum / (int64_t)positionSampleCount);

            packet.speed = (uint16_t)(gps.speed.kmph() * 100.0);

            packet.satellites = gps.satellites.value();

            packet.hdop = gps.hdop.value();

            packet.utc_time = gps.time.hour() * 10000UL + gps.time.minute() * 100UL + gps.time.second();

            if (gpsQueue != nullptr)
            {
                xQueueOverwrite(gpsQueue, &packet);
            }
        }

        if (millis() - lastDiagnosticMs >= 5000) {
            lastDiagnosticMs = millis();
            Serial.printf("[GPS] chars=%lu checksumOK=%lu checksumFail=%lu fix=%u age=%lu sat=%lu lat=%.6f lon=%.6f\n",
                gps.charsProcessed(), gps.passedChecksum(), gps.failedChecksum(),
                gps.location.isValid(), gps.location.age(), gps.satellites.value(),
                gps.location.lat(), gps.location.lng());
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
