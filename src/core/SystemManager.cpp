#include "TrackerConfig.h"

#include "SystemManager.h"
#include "tasks/topics.h"

#include "IMU/IMUDriver.h"
#include "IMU/ActivityFeature.h"
#include "IMU/ActivityTask.h"

#include "LoRa/LoRaDriver.h"
#include "tasks/Gps2LoraTask.h"


// ============================================================
// System Init
// ============================================================

void systemInit()
{
    bool radioReady = false;
    // ========================================================
    // Basic Hardware
    // ========================================================

    oledInit();
    // No external 1602 LCD in this hardware profile.


    // ========================================================
    // Create Queue
    // ========================================================

    serialQueue = xQueueCreate(
        10,
        sizeof(AppMessage)
    );

    wifiQueue = xQueueCreate(
        5,
        sizeof(AppMessage)
    );

    lcdQueue = xQueueCreate(
        10,
        sizeof(AppMessage)
    );

    oledQueue = xQueueCreate(
        10,
        sizeof(AppMessage)
    );

    mqttTxQueue = xQueueCreate(
        20,
        sizeof(AppMessage)
    );

    mqttRxQueue = xQueueCreate(
        20,
        sizeof(AppMessage)
    );

    otaQueue = xQueueCreate(
        10,
        sizeof(AppMessage)
    );

    statusQueue = xQueueCreate(
        10,
        sizeof(AppMessage)
    );


    // ========================================================
    // GPS Queue
    // ========================================================

    gpsQueue = xQueueCreate(
        1,
        sizeof(GPS_packet)
    );

    if (gpsQueue == nullptr)
    {
        Serial.println(
            "[SYSTEM] gpsQueue CREATE FAIL"
        );
    }
    else
    {
        Serial.println(
            "[SYSTEM] gpsQueue CREATE OK"
        );
    }


    // ========================================================
    // GPS Init
    // ========================================================

    gpsInit();


    // ========================================================
    // LoRa Init
    // ========================================================

    Serial.println(
        "[SYSTEM] Starting LoRa..."
    );

    if (TRACKER_LORA_ENABLED && (radioReady = LoRa.begin()))
    {
        Serial.println(
            "[SYSTEM] LoRa BEGIN OK"
        );
    }
    else
    {
        Serial.println(
            "[SYSTEM] LoRa disabled or BEGIN FAIL"
        );
    }


    // ========================================================
    // GPS → LoRa Init
    // ========================================================

    if (radioReady) gps2LoraInit();


    // ========================================================
    // IMU Init
    // ========================================================

    if (TRACKER_IMU_ENABLED && IMU.begin())
    {
        Serial.println(
            "[SYSTEM] IMU BEGIN OK"
        );
    }
    else
    {
        Serial.println(
            "[SYSTEM] IMU disabled or BEGIN FAIL"
        );
    }


    // ========================================================
    // Activity Shared Data
    // ========================================================

    activitySharedInit();

    activityTaskInit();


    // ========================================================
    // Topic Subscribe
    // ========================================================

    gBus.subscribe(
        "system/wifi",
        wifiQueue
    );

    gBus.subscribe(
        "system/mqtt",
        serialQueue
    );

    gBus.subscribe(
        "system/wifi",
        serialQueue
    );


    // --------------------------------------------------------
    // LCD
    // --------------------------------------------------------

    gBus.subscribe(
        "system/mqtt",
        lcdQueue
    );

    gBus.subscribe(
        "system/wifi",
        lcdQueue
    );


    // --------------------------------------------------------
    // OLED
    // --------------------------------------------------------

    gBus.subscribe(
        "system/mqtt",
        oledQueue
    );

    gBus.subscribe(
        "system/wifi",
        oledQueue
    );


    // --------------------------------------------------------
    // Serial
    // --------------------------------------------------------

    // --------------------------------------------------------
    // Status / MQTT
    // --------------------------------------------------------

    gBus.subscribe(
        "status/",
        statusQueue
    );

    gBus.subscribe(
        "device/",
        mqttTxQueue
    );


    // --------------------------------------------------------
    // Topics
    // --------------------------------------------------------

    gBus.subscribe(
        Topics::OTA_STATUS,
        mqttTxQueue
    );


    // --------------------------------------------------------
    // OTA
    // --------------------------------------------------------

    gBus.subscribe(
        Topics::OTA_UPDATE,
        otaQueue
    );

    gBus.subscribe(
        Topics::OTA_STATUS,
        otaQueue
    );


    // ========================================================
    // Create Tasks
    // ========================================================

    // --------------------------------------------------------
    // WiFi
    // --------------------------------------------------------

    if (TRACKER_NETWORK_ENABLED) {
    xTaskCreatePinnedToCore(
        wifiStateTask,
        "wifiStateTask",
        4096,
        NULL,
        4,
        NULL,
        0
    );
    }


    // --------------------------------------------------------
    // MQTT
    // --------------------------------------------------------

    if (TRACKER_NETWORK_ENABLED) {
    xTaskCreatePinnedToCore(
        mqttStateTask,
        "mqttStateTask",
        4096,
        NULL,
        3,
        NULL,
        0
    );
    }


    // --------------------------------------------------------
    // LCD
    // --------------------------------------------------------

    if (false) {
    xTaskCreatePinnedToCore(
        lcdTask,
        "lcdTask",
        4096,
        NULL,
        1,
        NULL,
        1
    );
    }


    // --------------------------------------------------------
    // OLED
    // --------------------------------------------------------

    if (TRACKER_TFT_ENABLED) {
    xTaskCreatePinnedToCore(
        oledTask,
        "oledTask",
        4096,
        NULL,
        1,
        NULL,
        1
    );
    }


    // ========================================================
    // GPS Task
    // ========================================================

    BaseType_t gpsRet =
        xTaskCreatePinnedToCore(
            gpsTask,
            "gpsTask",
            4096,
            NULL,
            1,
            NULL,
            1
        );

    if (gpsRet == pdPASS)
    {
        Serial.println(
            "[SYSTEM] gpsTask CREATE OK"
        );
    }
    else
    {
        Serial.println(
            "[SYSTEM] gpsTask CREATE FAIL"
        );
    }


    // ========================================================
    // GPS + Activity → LoRa
    //
    // IMPORTANT:
    //
    // 不再建立 imu2LoraTask
    //
    // gps2LoraTask 負責：
    //
    // GPS + latestActivityFeatures
    //             ↓
    //       DogStatusPayload
    //             ↓
    //          TYPE = 3
    //             ↓
    //           LoRa
    // ========================================================

    BaseType_t gpsLoraRet = pdFAIL;
    if (radioReady) gpsLoraRet =
        xTaskCreatePinnedToCore(
            gps2LoraTask,
            "gps2LoraTask",
            4096,
            NULL,
            1,
            NULL,
            1
        );

    if (gpsLoraRet == pdPASS)
    {
        Serial.println(
            "[SYSTEM] gps2LoraTask CREATE OK"
        );
    }
    else
    {
        Serial.println(
            "[SYSTEM] gps2LoraTask disabled, radio unavailable, or CREATE FAIL"
        );
    }


    // ========================================================
    // Activity Task
    //
    // BMI270
    //   ↓
    // IMU.read()
    //   ↓
    // ActivityFeature
    //   ↓
    // ActivityScore
    //   ↓
    // latestActivityFeatures
    // ========================================================

    BaseType_t activityRet = pdFAIL;
    if (TRACKER_IMU_ENABLED && IMU.isReady()) activityRet =
        xTaskCreatePinnedToCore(
            activityTask,
            "activityTask",
            4096,
            NULL,
            1,
            NULL,
            1
        );

    if (activityRet == pdPASS)
    {
        Serial.println(
            "[SYSTEM] activityTask CREATE OK"
        );
    }
    else
    {
        Serial.println(
            "[SYSTEM] activityTask disabled, IMU unavailable, or CREATE FAIL"
        );
    }


    // ========================================================
    // Serial
    // ========================================================

    xTaskCreatePinnedToCore(
        serialTask,
        "serialTask",
        4096,
        NULL,
        2,
        NULL,
        1
    );


    // ========================================================
    // Status
    // ========================================================

    if (TRACKER_NETWORK_ENABLED) {
    xTaskCreatePinnedToCore(
        statusTask,
        "StatusTask",
        10240,
        NULL,
        1,
        NULL,
        1
    );
    }


    // ========================================================
    // OTA
    // ========================================================

    if (TRACKER_NETWORK_ENABLED) {
    xTaskCreatePinnedToCore(
        otaTask,
        "OTA",
        8192,
        NULL,
        3,
        NULL,
        1
    );
    }


    // ========================================================
    // System Init Complete
    // ========================================================

    Serial.println(
        "[SYSTEM] systemInit COMPLETE"
    );
}
