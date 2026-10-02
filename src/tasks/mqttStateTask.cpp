
#include "MqttStateTask.h"
#include "tasks/topics.h"

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);
// static QueueHandle_t mqttQueue;
const char *broker = "cherry-garden.idv.tw";
const char *MQTTUser = "jjchyuan";       // 帳
const char *MQTTPassword = "openjjc543"; // 密

QueueHandle_t mqttTxQueue;
QueueHandle_t mqttRxQueue;
QueueHandle_t wifiQueue;
TaskHandle_t mqttHandle = NULL;

void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    AppMessage msg;

    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.topic[sizeof(msg.topic) - 1] = 0;

    if (len >= sizeof(msg.payload))
        len = sizeof(msg.payload) - 1;

    memcpy(msg.payload, payload, len);
    msg.payload[len] = 0;

    // xQueueSend(mqttRxQueue, &msg, 0);

    xQueueSend(mqttRxQueue, &msg, pdMS_TO_TICKS(100));
}

enum class MqttState
{
    WAIT_WIFI,
    CONNECTING,
    CONNECTED
};

void mqttStateTask(void *pv)
{

    mqtt.setServer(broker, 1883);
    mqtt.setCallback(mqttCallback);
    mqtt.setBufferSize(1024);// 🔥 加這行

    AppMessage msg;
    MqttState state = MqttState::WAIT_WIFI;

    uint32_t lastTry = 0;

    while (1)
    {
        switch (state)
        {
        // ⭐ 等 WiFi 上線
        case MqttState::WAIT_WIFI:
            if (xQueueReceive(wifiQueue, &msg, 0))
            {
                if (!strcmp(msg.payload, "connected"))
                    state = MqttState::CONNECTING;
            }
            break;

        // ⭐ 連線 broker（有 backoff）
        case MqttState::CONNECTING:
            if (millis() - lastTry > 5000)
            {
                lastTry = millis();
                String MQTTClientid = "esp32-" + String(random(1000000, 9999999));

                if (mqtt.connect(MQTTClientid.c_str(), MQTTUser, MQTTPassword))
                {
                    mqtt.subscribe(Topics::OTA_UPDATE); // 訂閱 OTA 更新指令

                    // mqtt.subscribe(Topics::OTA_STATUS);

                    strcpy(msg.topic, "system/mqtt");
                    strcpy(msg.payload, "connected");
                    gBus.publish(msg);

                    state = MqttState::CONNECTED;
                }
            }
            break;

        // ⭐ 正常運行
        case MqttState::CONNECTED:

            // 必須持續呼叫 loop
            mqtt.loop();

            // TX queue → publish
            while (xQueueReceive(mqttTxQueue, &msg, 0))
            {
                mqtt.publish(msg.topic, msg.payload);
            }

            // RX queue → 丟 EventBus
            while (xQueueReceive(mqttRxQueue, &msg, 0))
                gBus.publish(msg);

            // 若 broker 掉線
            if (!mqtt.connected())
            {
                strcpy(msg.topic, "system/mqtt");
                strcpy(msg.payload, "lost");
                gBus.publish(msg);

                state = MqttState::CONNECTING;
            }
            break;
        }

        // ⭐ Watchdog 安全點
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ===== API =====

void mqttPublish(const char *topic, const char *msg)
{
    if (mqtt.connected())
        mqtt.publish(topic, msg);
}

bool mqttIsConnected()
{
    return mqtt.connected();
}

void mqttSuspend()
{
    if (mqttHandle)
    {
        Serial.println("MQTT suspend");
        vTaskSuspend(mqttHandle);
    }
}

void mqttResume()
{
    if (mqttHandle)
    {
        Serial.println("MQTT resume");
        vTaskResume(mqttHandle);
    }
}

void mqttStop()
{
    if (mqtt.connected())
        mqtt.disconnect();
}
