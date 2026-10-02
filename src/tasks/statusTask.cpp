#include "statusTask.h"
#include <WiFi.h>
#include <ArduinoJson.h>

// ===== 外部 EventBus =====
// extern EventBus gBus;

// ===== Queue =====
QueueHandle_t statusQueue;

String getDeviceId()
{
    uint64_t chipid = ESP.getEfuseMac();
    char id[32];
    snprintf(id, sizeof(id), "esp32-%04X%08X",
             (uint16_t)(chipid >> 32),
             (uint32_t)chipid);
    return String(id);
}

// ===== JSON 建構 =====


static size_t buildStatusJson(char *out, size_t maxSize)
{
    StaticJsonDocument<768> doc;

    char ip[16];
    WiFi.localIP().toString().toCharArray(ip, 16);
    doc["ip"] = ip;

    doc["id"] = getDeviceId();
    doc["name"] = "Light2_timer_LCD_ota_162:wc";
    doc["mqtt32_pub"] = "device/id/status";
    doc["web_sub"] = "device/#";
    doc["update32"] = Topics::OTA_UPDATE;
    doc["fw"] = FW_VERSION;
    doc["rssi"] = WiFi.RSSI();

    String ssid;
    if (WiFi.isConnected())
        ssid = WiFi.SSID();
    doc["ssid"] = ssid;

    doc["up"] = millis() / 1000;
   doc["heap"] = ESP.getFreeHeap();
    doc["wifi"] = (WiFi.status() == WL_CONNECTED) ? 1 : 0;

    // 🔥 1. 計算實際需要長度
    size_t needed = measureJson(doc);

    // 🔥 2. 檢查是否超過 buffer
    if (needed >= maxSize)
    {
        Serial.printf("❌ JSON overflow! need=%d, buf=%d\n", needed, maxSize);
        return 0;
    }

    // 🔥 3. 正式輸出
    return serializeJson(doc, out, maxSize);
}

// ===== 發佈 status =====
static void publishStatus()
{
    AppMessage msg6;

    String deviceId = getDeviceId();

    snprintf(msg6.topic, sizeof(msg6.topic),
             "device/%s/status", deviceId.c_str());

    buildStatusJson(msg6.payload, sizeof(msg6.payload));

    gBus.publish(msg6);
}

// ===== heartbeat =====
static void publishHeartbeat()
{
    AppMessage msg7;
    String deviceId = getDeviceId();
    snprintf(msg7.topic, sizeof(msg7.topic),
             "device/%s/heartbeat", deviceId.c_str());

    snprintf(msg7.payload, sizeof(msg7.payload),
             "{\"online\":1}");

    gBus.publish(msg7);
}

// ===== 對外 API =====
void statusRequest(const char *reason)
{
    AppMessage msg8;

    snprintf(msg8.topic, sizeof(msg8.topic), "status/req");

    snprintf(msg8.payload, sizeof(msg8.payload),
             "{\"r\":\"%s\"}", reason ? reason : "unknown");

    gBus.publish(msg8);
}

// ===== Task 主體 =====
void statusTask(void *pvParameters)
{
    AppMessage msg9;
    uint32_t lastHb = 0;

    while (1)
    {
        if (xQueueReceive(statusQueue, &msg9, 1000 / portTICK_PERIOD_MS))
        {
            // ===== routing =====
            if (strncmp(msg9.topic, "status/req", 10) == 0)
            {
                publishStatus();
            }
            else if (strncmp(msg9.topic, "status/event", 12) == 0)
            {
                publishStatus();
            }
        }

        // ===== heartbeat =====
        if (millis() - lastHb > 3000)
        {
            lastHb = millis();
            publishHeartbeat();
            publishStatus(); // 👈 加這行

        }
    }
}