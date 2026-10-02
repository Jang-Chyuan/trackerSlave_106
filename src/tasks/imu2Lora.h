#pragma once

#include <Arduino.h>

#include "LoRa/LoRaPacket.h"


// ============================================================
// IMU Activity LoRa Packet Type
// ============================================================
//
// GPS      = TYPE 1
// Activity = TYPE 2
//

#ifndef LORA_TYPE_ACTIVITY
#define LORA_TYPE_ACTIVITY 2
#endif


// ============================================================
// Activity Payload
// ============================================================
//
// activityScore:
//
//     0.000 ~ 1.000
//
// LoRa:
//
//     0    = 0.000
//     500  = 0.500
//     1000 = 1.000
//
// 使用 uint16_t，不直接傳 float。
// Payload size = 2 bytes
//
// ============================================================

struct IMUActivityPayload
{
    uint16_t activityScore;
};


// ============================================================
// API
// ============================================================

void imu2LoraInit();

bool imu2LoraSend(
    float activityScore
);

