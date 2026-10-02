#pragma once

#include <Arduino.h>
#include "../core/AppMessage.h"
#include "TrackerDisplay.h"
#include "../LoRa/LoRaPacket.h"

extern TrackerDisplay display;
extern QueueHandle_t oledQueue;

void oledInit(void);
void oledShowTransmission(const DogStatusPayload &payload, uint16_t sequence,
                          bool gpsAvailable, bool success);
void oledTask(void *pv);
