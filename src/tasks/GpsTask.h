#pragma once

#include <Arduino.h>
#include "../core/EventBus.h"
#include "gps.h"
#include "HT_TinyGPS++.h"

extern QueueHandle_t gpsQueue;

void gpsON(void);
void gpsTask(void *pv);
void gpsInit(void);


struct GPS_packet
{
    uint32_t capturedAtMs; // millis() at valid location reception.
    uint32_t updateId;     // GPS更新序號
    int32_t lat;
    int32_t lon;
    uint16_t speed;
    uint8_t satellites;
    uint16_t hdop;
    uint32_t utc_time;
};

