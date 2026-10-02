#pragma once
#include <freertos/queue.h>
#include "../core/AppMessage.h"
#include "../core/EventBus.h"
extern QueueHandle_t otaQueue;

void otaTask(void *pv);
void initOTATask();