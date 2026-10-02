#include <WiFi.h>
#include <HTTPUpdate.h>
#include "config/config.h"
#include "ota_task.h"
#include "mqttStateTask.h"
#include "tasks/topics.h"

WiFiClient otaClient;
QueueHandle_t otaQueue;
const char *firmwareUrl = "http://1.34.84.15:3300/ota/fw.bin";
// const char *firmwareUrl = "http://ota.cherry-garden.idv.tw/ota/fw.bin";

void doOTA()
{

  Serial.println("=== OTA START ===");

  mqttSuspend();
  mqttStop();

  vTaskDelay(pdMS_TO_TICKS(200));

  WiFi.setSleep(false);

  Serial.println("Start OTA...");
  t_httpUpdate_return ret = httpUpdate.update(otaClient, firmwareUrl);

  if (ret == HTTP_UPDATE_OK)
  {
    Serial.println("OTA OK → reboot");
  }
  else
  {
    Serial.printf("OTA FAIL: %s\n",
                  httpUpdate.getLastErrorString().c_str());

    mqttPublish(Topics::OTA_STATUS, "ota_fail");
    mqttResume();
  }
}

void otaTask(void *pv)
{

  AppMessage msg5;

  // int cmd;

  while (1)
  {
    if (xQueueReceive(otaQueue, &msg5, portMAX_DELAY))
    {
      if (strcmp(msg5.topic, Topics::OTA_UPDATE) == 0 && strcmp(msg5.payload, "start") == 0)
      
        snprintf(msg5.topic, sizeof(msg5.topic), Topics::OTA_STATUS);
        snprintf(msg5.payload, sizeof(msg5.payload), "%s", "received ota_update_cmd");
        gBus.publish(msg5);
        vTaskDelay(pdMS_TO_TICKS(200));

      {
        doOTA();
      }
    }
  }
  vTaskDelay(pdMS_TO_TICKS(10));
}