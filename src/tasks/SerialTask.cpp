#include "SerialTask.h"
#include "../core/EventBus.h"

QueueHandle_t serialQueue = NULL;


void serialTask(void *pv)
{
    AppMessage msg;

    // 等待 USB CDC 初始化
    vTaskDelay(pdMS_TO_TICKS(2000));

    Serial.println();
    Serial.println("===== Serial Task Start =====");
    Serial.flush();


    while (1)
    {
        if (xQueueReceive(serialQueue, &msg, portMAX_DELAY))
        {
            Serial.print("[Serial] ");
            Serial.print(msg.topic);
            Serial.print(" : ");
            Serial.println(msg.payload);

            // 確保 USB CDC 資料送出
            Serial.flush();
        }
    }
}