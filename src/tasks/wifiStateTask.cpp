#include "WifiStateTask.h"




/* ===============================
   Helper: Publish Event
================================ */


static void publishStatus(const char *text) {
  AppMessage msg;
  snprintf(msg.topic, sizeof(msg.topic), "system/wifi");
  snprintf(msg.payload, sizeof(msg.payload), text);
  gBus.publish(msg);
}

/* ===============================
   Select Best RSSI
================================ */

static int selectBestWiFi() {
  int n = WiFi.scanNetworks();
  if (n <= 0)
    return -1;

  int best = -1;
  int bestRSSI = -999;

  for (int i = 0; i < n; i++) {
    String found = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);

    for (int j = 0; j < wifiCount; j++) {
      if (found == wifiList[j].ssid) {
        if (rssi > bestRSSI && rssi > -80)  // RSSI下限
        {
          bestRSSI = rssi;
          best = j;
        }
      }
    }
  }

  WiFi.scanDelete();
  return best;
}

/* ===============================
   WiFi Event Callback
================================ */

static void WiFiEvent(WiFiEvent_t event) {
  AppMessage msg;
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      publishStatus("connected");

      //publishStatus("connected");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      publishStatus("got_ip");
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      publishStatus("disconnected");
      state = WifiState::LOST;
      WiFi.reconnect();  // 系統會嘗試重連
      break;

    default:
      break;
  }
}

/* ===============================
   Main WiFi Task
================================ */

void wifiStateTask(void *pv) {
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);  // 我們自己管理
  WiFi.persistent(false);

  while (1) {

    
    switch (state) {
      case WifiState::INIT:
        publishStatus("init");
        state = WifiState::SCANNING;
        break;

      case WifiState::SCANNING:
        publishStatus("scanning");
        bestIndex = selectBestWiFi();
        if (bestIndex >= 0) {
          state = WifiState::CONNECTING;
        } else {
          vTaskDelay(pdMS_TO_TICKS(5000));
        }
        break;

      case WifiState::CONNECTING:
        publishStatus("connecting");
        WiFi.begin(wifiList[bestIndex].ssid,
                   wifiList[bestIndex].password);

        connectStart = millis();
        state = WifiState::CONNECTED;
        Serial.println("wifi: ");
        Serial.println("CONNECTED");

        break;

      case WifiState::CONNECTED:
        if (WiFi.status() != WL_CONNECTED) {
          if (millis() - connectStart > 10000) {
            state = WifiState::LOST;
          }
        }
        break;

      case WifiState::LOST:
        WiFi.disconnect();
        vTaskDelay(pdMS_TO_TICKS(1000));
        state = WifiState::SCANNING;
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}