#include "topics.h"

namespace Topics
{
     // 推播主題

    const char *OTA_STATUS = "dogLoraSlave_2/ota/status"; // ota 狀態
    // 訂閱主題

    const char *OTA_UPDATE = "dogLoraSlave_2/ota/update"; // ota 更新指令

    //   ESP32mqtt_publish  ="device/getDeviceId()/status" per 5 seconds
    //   ESP32web_subscribe  ="device/#"
}
