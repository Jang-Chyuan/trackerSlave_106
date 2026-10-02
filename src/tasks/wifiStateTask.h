#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <WiFi.h>
#include "../core/EventBus.h"

//EventBus gBus;

/* ===============================
   WiFi Credential List
================================ */

struct WifiCredential {
  const char *ssid;
  const char *password;
};

static WifiCredential wifiList[] = {
  { "J_office", "01200401" },
  { "JJC2f_2G", "01200401" },
  { "Chyuan_Dlink_2F", "01200401" },
 { "perfume_Garden_2G", "01200401" },
  { "Ganden_TOLINK_2G", "01200401" },
  { "J_workshop2", "01200401" },
  { "Workshop3", "01200401" },
  { "windy_tim", "01200401" },
  { "greenhouse_333", "01200401" }
};

static const int wifiCount =
  sizeof(wifiList) / sizeof(wifiList[0]);

/* ===============================
   State Machine
================================ */

enum class WifiState {
  INIT,
  SCANNING,
  CONNECTING,
  CONNECTED,
  LOST
};

static WifiState state = WifiState::INIT;
static int bestIndex = -1;
static unsigned long connectStart = 0;

extern QueueHandle_t wifiQueue;

void wifiStateTask(void* pv);