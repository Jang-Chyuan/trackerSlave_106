#pragma once
#include <Arduino.h>
#include "../core/AppMessage.h"
#include <LiquidCrystal_PCF8574.h>
extern QueueHandle_t lcdQueue;

void led1602_init();
void lcdTask(void* pv);
