
/*

GPS 與 gpsQueue 完全獨立，不要依賴 AppMessage，如下架構：

gpsTask
    │
    │ GPS_PACKET
    ▼
gpsQueue
    │
    ├── LoRaTask
    ├── MQTTTask
    └── OLEDTask (若需要顯示)

*/



#pragma once
#include <Arduino.h>

struct AppMessage
{
    char topic[64];
    char payload[768];
};






