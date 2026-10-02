#include "imu2Lora.h"

#include "IMU/ActivityFeature.h"
#include "LoRa/LoRaDriver.h"


// ============================================================
// LoRa Driver
// ============================================================

extern LoRaDriver LoRa;


// ============================================================
// Sequence number
// ============================================================

static uint16_t activitySeq = 0;


// ============================================================
// Last Activity Result Timestamp
// ============================================================
//
// ActivityFeature 每完成一個 10 sec window
// timestamp 會更新一次。
//
// imu2Lora 使用 timestamp 判斷是否為新的 Activity Result。
//
// 因此：
//     每個 10 sec window 只送一次。
// ============================================================

static uint32_t lastActivityTimestamp = 0;


// ============================================================
// Init
// ============================================================

void imu2LoraInit()
{
    activitySeq = 0;

    lastActivityTimestamp = 0;

    Serial.println("[IMU2LORA] Init");
}


// ============================================================
// Send Activity Score
// ============================================================
//
// Input:
//     activityScore = 0.0 ~ 1.0
//
// LoRa payload:
//     uint16_t
//
// Mapping:
//     0.000 -> 0
//     0.500 -> 500
//     1.000 -> 1000
//
// ============================================================

bool imu2LoraSend(float activityScore)
{
    // --------------------------------------------------------
    // Clamp 0~1
    // --------------------------------------------------------

    if (activityScore < 0.0f)
    {
        activityScore = 0.0f;
    }

    if (activityScore > 1.0f)
    {
        activityScore = 1.0f;
    }


    // --------------------------------------------------------
    // Convert float -> uint16_t
    //
    // 0.000 ~ 1.000
    //        ↓
    // 0 ~ 1000
    // --------------------------------------------------------

    uint16_t score =
        static_cast<uint16_t>(
            activityScore * 1000.0f + 0.5f
        );


    // --------------------------------------------------------
    // Payload
    // --------------------------------------------------------

    IMUActivityPayload payload;

    payload.activityScore = score;


    // --------------------------------------------------------
    // LoRa Packet
    // --------------------------------------------------------

    LoRaPacket pkt;

    pkt.type = LORA_TYPE_ACTIVITY;

    pkt.seq = activitySeq++;

    pkt.len = sizeof(IMUActivityPayload);


    // --------------------------------------------------------
    // Copy payload
    // --------------------------------------------------------

    memcpy(
        pkt.payload,
        &payload,
        sizeof(IMUActivityPayload)
    );


    // --------------------------------------------------------
    // Send
    // --------------------------------------------------------

    bool ok = LoRa.send(pkt);


    // --------------------------------------------------------
    // Debug
    // --------------------------------------------------------

    Serial.print("[IMU2LORA] ");

    if (ok)
    {
        Serial.print("TX OK");
    }
    else
    {
        Serial.print("TX FAIL");
    }

    Serial.print(" TYPE=");
    Serial.print(pkt.type);

    Serial.print(" SEQ=");
    Serial.print(pkt.seq);

    Serial.print(" SCORE=");
    Serial.print(activityScore, 3);

    Serial.print(" VALUE=");
    Serial.println(score);


    return ok;
}


// ============================================================
// IMU -> LoRa Task
// ============================================================
//
// 每 100 ms 檢查一次最新 Activity Result。
//
// ActivityFeature:
//     100 Hz
//     1000 samples
//     10 sec
//
// 每完成一個 window：
//     timestamp 改變
//
// imu2Lora:
//     timestamp 改變
//          ↓
//     send Activity Score
//
// 所以每 10 sec 只送一次。
// ============================================================

