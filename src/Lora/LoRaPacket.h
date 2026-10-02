#pragma once

#include <Arduino.h>


// ============================================================
// LoRa Packet Type
// ============================================================

#ifndef LORA_TYPE_PING
#define LORA_TYPE_PING        1
#endif

#ifndef LORA_TYPE_ACTIVITY
#define LORA_TYPE_ACTIVITY    2
#endif

#ifndef LORA_TYPE_DOG_STATUS
#define LORA_TYPE_DOG_STATUS  3
#endif

#ifndef LORA_TYPE_POLL
#define LORA_TYPE_POLL        4
#endif


// ============================================================
// Maximum LoRa payload
// ============================================================

#ifndef LORA_MAX_PAYLOAD
#define LORA_MAX_PAYLOAD 32
#endif


// ============================================================
// LoRa Packet
// ============================================================

struct LoRaPacket
{
    uint8_t type;

    uint16_t seq;

    uint8_t len;

    uint8_t payload[LORA_MAX_PAYLOAD];
};

// TYPE = 4. A slave responds only when both IDs match its configured network.
struct __attribute__((packed)) PollPayload
{
    uint8_t masterId;
    uint8_t targetSlaveId;
};

static_assert(sizeof(PollPayload) == 2, "PollPayload must be exactly 2 bytes");


// ============================================================
// DOG STATUS Payload
//
// TYPE = 3
//
// GPS + Activity Score + Battery Status
//
// TOTAL = 30 bytes; 29-byte packets remain valid for backward compatibility.
// No new valid GPS: lat/lon/speed/satellites/gpsTimestamp = 0,
// hdop = UINT16_MAX. Receivers must treat this as unavailable GPS.
// ============================================================

struct __attribute__((packed)) DogStatusPayload
{
    // --------------------------------------------------------
    // GPS
    // --------------------------------------------------------

    int32_t  lat;

    int32_t  lon;

    uint16_t speed;

    uint8_t  satellites;

    uint16_t hdop;

    uint32_t gpsTimestamp;


    // --------------------------------------------------------
    // Activity
    // --------------------------------------------------------

    uint16_t activityScore;

    uint8_t  activityValid;

    uint32_t activityTimestamp;

    // --------------------------------------------------------
    // Battery
    // --------------------------------------------------------

    uint16_t batteryMillivolts;

    uint8_t  batteryPercentage;

    uint8_t  batteryValid;

    // Sender identity.
    uint8_t  slaveId;

    // USB 5V present through the GPIO47 resistor divider.
    uint8_t  usbPresent;
};


// ============================================================
// Compile-time check
// ============================================================

static_assert(
    sizeof(DogStatusPayload) == 30,
    "DogStatusPayload must be exactly 30 bytes"
);
