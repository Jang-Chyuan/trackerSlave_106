#include <WiFi.h>
#include "esp_ota_ops.h"
#include "health_check.h"

bool systemOK() {

  if (!WiFi.isConnected()) return false;

  return true;
}

void runHealthCheck() {

  const esp_partition_t* running = esp_ota_get_running_partition();
  esp_ota_img_states_t state;

  if (esp_ota_get_state_partition(running, &state) == ESP_OK) {

    if (state == ESP_OTA_IMG_PENDING_VERIFY) {

      Serial.println("Checking firmware...");

      if (systemOK()) {
        esp_ota_mark_app_valid_cancel_rollback();
        Serial.println("✅ VALID");

      } else {
        Serial.println("❌ INVALID → rollback");
        esp_restart();
      }
    }
  }
}