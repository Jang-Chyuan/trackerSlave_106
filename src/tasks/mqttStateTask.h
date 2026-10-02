

#pragma once
#include <PubSubClient.h>
#include <WiFi.h>
#include "../core/AppMessage.h"
#include "../core/EventBus.h"
#include "../tasks/wifiStateTask.h"

extern QueueHandle_t mqttTxQueue;
extern QueueHandle_t mqttRxQueue;
extern TaskHandle_t mqttHandle;

void mqttStateTask(void* pv);
void mqttCallback(char *topic, byte *payload, unsigned int len);

void mqttPublish(const char* topic, const char* msg);
bool mqttIsConnected();

void mqttSuspend();
void mqttResume();
void mqttStop();