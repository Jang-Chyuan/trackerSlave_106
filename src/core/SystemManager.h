#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "core/EventBus.h"
#include "core/AppMessage.h"
#include "tasks/wifiStateTask.h"
#include "tasks/MqttStateTask.h"
#include "tasks/SerialTask.h"


#include "tasks/ota_task.h"
#include "tasks/statusTask.h"

#include "tasks/LcdTask.h"
#include "tasks/OledTask.h"
#include "tasks/GpsTask.h"
#include "tasks/Gps2LoraTask.h"
#include "IMU/ActivityFeature.h"
#include "IMU/ActivityTask.h"
#include "tasks/imu2Lora.h"
void systemInit();



/*

void imu2LoraInit();

bool imu2LoraSend(float activityScore);

void imu2LoraTask(void *pvParameters);



*/
