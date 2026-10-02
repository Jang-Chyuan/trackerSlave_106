#pragma once
#include <Arduino.h>
#include "../core/EventBus.h"
#include "../core/AppMessage.h"


extern QueueHandle_t buttonQueue;
extern QueueHandle_t remoteQueue;

void ButtonTask(void *pv);
void button_ini();
void Read_Onoff_Button();
void Read_Up_Button();
void Read_Down_Button();
void Read_Start_Button();

