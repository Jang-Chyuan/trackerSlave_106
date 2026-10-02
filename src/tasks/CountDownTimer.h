#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Countimer.h>
#include "../core/AppMessage.h"
#include "../core/EventBus.h"
#include "../tasks/ButtonTask.h"
extern QueueHandle_t timerQueue;
extern QueueHandle_t relay13Queue;





void tDown13Complete();
void local_countDown13();
void mqtt_countDown13();
void print13_time();
void CountDownTimerTask(void* pv);