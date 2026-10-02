#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../core/AppMessage.h"
#include "../core/EventBus.h"
#include "../tasks/topics.h"

// ===== Device Info =====
//#define DEVICE_ID   "NGarden"
#define FW_VERSION  "1.0.3"

// ===== Queue =====
extern QueueHandle_t statusQueue;

// ===== Task =====
void statusTask(void *pvParameters);

// ===== API（給其他 task 呼叫）=====
void statusRequest(const char* reason);