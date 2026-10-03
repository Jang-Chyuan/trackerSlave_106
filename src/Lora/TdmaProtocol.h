#pragma once
#include <stdint.h>

constexpr uint8_t LORA_TYPE_SYNC = 5;
constexpr uint32_t TDMA_FRAME_MS = 10000;
constexpr uint32_t TDMA_SYNC_LEAD_MS = 2000;
constexpr uint32_t TDMA_MAX_LATE_MS = 80;
constexpr uint32_t TDMA_SYNC_TIMEOUT_MS = 30000;
constexpr uint32_t TDMA_AUTONOMOUS_JITTER_MS = 400;
constexpr uint32_t TDMA_AUTONOMOUS_WINDOW_MS = 2800;
constexpr uint32_t TDMA_CAD_TIMEOUT_MS = 300;

// epochMs is the upcoming frame start in Master 9 millis(), not UTC.
// Driver fills remainingMs immediately before TX, compensating airtime.
struct __attribute__((packed)) SyncPayload
{
    uint8_t masterId;
    uint32_t epochMs;
    uint32_t frame;
    uint32_t remainingMs; // time from RX_DONE to epochMs
};
static_assert(sizeof(SyncPayload) == 13, "SYNC wire size");

constexpr uint32_t tdmaSlotMs(uint8_t id)
{
    return id == 4 ? 0 : id == 6 ? 3000 : id == 8 ? 6000 : id == 108 ? 8000 : UINT32_MAX;
}

inline int32_t tdmaDelta(uint32_t now, uint32_t target)
{
    return static_cast<int32_t>(now - target);
}
