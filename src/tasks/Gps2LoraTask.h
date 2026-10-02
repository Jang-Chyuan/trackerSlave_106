#pragma once

#include <Arduino.h>
#include "GpsTask.h"
#include "LoRa/LoRaPacket.h"


// ============================================================
// GPS → LoRa Task
// ============================================================

void gps2LoraInit();
void gps2LoraHandlePacket(const LoRaPacket &packet, uint32_t receivedAtMs);
bool gps2LoraSyncPending();
void gps2LoraTask(void *pvParameters);
