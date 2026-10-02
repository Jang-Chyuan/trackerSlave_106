#include <Arduino.h>
#include <esp_system.h>
#include "LoRa/TdmaProtocol.h"

#include "HT_TinyGPS++.h"

#include "GpsTask.h"
#include "Gps2LoraTask.h"
#include "OledTask.h"
#include "batteryStatus.h"
#include "TrackerConfig.h"

#include "IMU/ActivityFeature.h"

#include "LoRa/LoRaDriver.h"
#include "LoRa/LoRaPacket.h"
#include "core/DeviceIdentity.h"


// ============================================================
// External LoRa
// ============================================================

extern LoRaDriver LoRa;
extern BatteryStatus batteryStatus;


// ============================================================
// Sequence
// ============================================================

struct SyncEvent
{
    SyncPayload sync;
    uint32_t receivedAtMs;
};
static QueueHandle_t syncQueue = nullptr;
static uint32_t lastSentGpsUpdateId = 0;
static constexpr uint32_t GPS_MAX_AGE_MS = 3000;
static_assert(tdmaSlotMs(SLAVE_DEVICE_ID) != UINT32_MAX, "Unknown TDMA slave");

void gps2LoraInit()
{
    if (syncQueue == nullptr) syncQueue = xQueueCreate(1, sizeof(SyncEvent));
    Serial.printf("[TDMA] slave=%u slot=%lu queue=%s\n", SLAVE_DEVICE_ID,
                  static_cast<unsigned long>(tdmaSlotMs(SLAVE_DEVICE_ID)),
                  syncQueue ? "OK" : "FAIL");
}

bool gps2LoraSyncPending()
{
    return syncQueue && uxQueueMessagesWaiting(syncQueue) != 0;
}

void gps2LoraHandlePacket(const LoRaPacket &packet, uint32_t receivedAtMs)
{
    if (!syncQueue || packet.type != LORA_TYPE_SYNC || packet.len != sizeof(SyncPayload)) return;
    SyncEvent event{};
    memcpy(&event.sync, packet.payload, sizeof(event.sync));
    if (event.sync.masterId != EXPECTED_MASTER_DEVICE_ID ||
        event.sync.remainingMs > TDMA_SYNC_LEAD_MS ||
        packet.seq != static_cast<uint16_t>(event.sync.frame)) return;
    event.receivedAtMs = receivedAtMs;
    xQueueOverwrite(syncQueue, &event);
}

// ============================================================
// Send GPS + Activity
// ============================================================

static bool sendDogStatus(
    const GPS_packet& gps,
    uint16_t responseSequence,
    bool gpsAvailable,
    uint32_t slotAtMs,
    bool autonomous,
    uint32_t jitterMs
)
{
    // --------------------------------------------------------
    // Get latest Activity
    // --------------------------------------------------------

    ActivityFeatures activity;

    bool activityOK =
        activityGetLatest(activity);


    // --------------------------------------------------------
    // Payload
    // --------------------------------------------------------

    DogStatusPayload payload{};


    // ========================================================
    // GPS
    // ========================================================

    payload.lat =
        gps.lat;

    payload.lon =
        gps.lon;

    payload.speed =
        gps.speed;

    payload.satellites =
        gps.satellites;

    payload.hdop =
        gps.hdop;

    payload.gpsTimestamp =
        gps.utc_time;


    // ========================================================
    // Activity
    // ========================================================

    if (
        activityOK &&
        activity.valid
    )
    {
        float score =
            activity.activityScore;

        if (score < 0.0f)
            score = 0.0f;

        if (score > 1.0f)
            score = 1.0f;


        payload.activityScore =
            static_cast<uint16_t>(
                score * 1000.0f + 0.5f
            );

        payload.activityValid =
            1;

        payload.activityTimestamp =
            activity.timestamp;
    }
    else
    {
        payload.activityScore = 0;

        payload.activityValid = 0;

        payload.activityTimestamp = 0;
    }


    // ========================================================
    // Battery
    // ========================================================

    const BatteryReading battery = batteryStatus.readFromAdc();

    payload.batteryMillivolts = battery.valid
        ? static_cast<uint16_t>(battery.voltage * 1000.0f + 0.5f)
        : 0;

    payload.batteryPercentage = battery.percentage;

    payload.batteryValid = battery.valid ? 1 : 0;

    payload.slaveId = SLAVE_DEVICE_ID;
    payload.usbPresent = digitalRead(TRACKER_USB_PRESENT) == HIGH ? 1 : 0;


    // ========================================================
    // LoRa Packet
    // ========================================================

    LoRaPacket pkt{};

    pkt.type =
        LORA_TYPE_DOG_STATUS;

    pkt.seq = responseSequence;

    pkt.len =
        sizeof(DogStatusPayload);


    memcpy(
        pkt.payload,
        &payload,
        sizeof(DogStatusPayload)
    );


    // ========================================================
    // ONLY print when TX is actually going to happen
    // ========================================================

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "[GPS2LORA] TX DOG STATUS"
    );

    Serial.print(
        "  TYPE="
    );

    Serial.println(
        pkt.type
    );

    Serial.print(
        "  SEQ="
    );

    Serial.println(
        pkt.seq
    );

    Serial.print(
        "  LAT="
    );

    Serial.println(
        payload.lat
    );

    Serial.print(
        "  LON="
    );

    Serial.println(
        payload.lon
    );

    Serial.print(
        "  SPEED="
    );

    Serial.print(
        payload.speed / 100.0f,
        2
    );

    Serial.println(
        " km/h"
    );

    Serial.print(
        "  SAT="
    );

    Serial.println(
        payload.satellites
    );

    Serial.print(
        "  HDOP="
    );

    Serial.println(
        payload.hdop
    );

    Serial.print(
        "  GPS_TIME="
    );

    Serial.println(
        payload.gpsTimestamp
    );

    Serial.print(
        "  ACTIVITY="
    );

    Serial.println(
        payload.activityScore / 1000.0f,
        3
    );

    Serial.print(
        "  ACTIVITY_VALID="
    );

    Serial.println(
        payload.activityValid
    );

    Serial.print(
        "  ACTIVITY_TIMESTAMP="
    );

    Serial.println(
        payload.activityTimestamp
    );


    // ========================================================
    // TX
    // ========================================================

    bool ok =
        LoRa.sendScheduled(pkt, slotAtMs, autonomous, jitterMs);

    oledShowTransmission(payload, pkt.seq, gpsAvailable, ok);


    if (ok)
    {
        Serial.println(
            "[GPS2LORA] TX OK"
        );
    }
    else
    {
        Serial.println(
            "[GPS2LORA] TX FAIL"
        );
    }

    Serial.println(
        "================================"
    );


    return ok;
}


// ============================================================
// GPS2LORA Task
// ============================================================

void gps2LoraTask(void* pvParameters)
{
    (void)pvParameters;
    bool haveSync = false;
    bool autonomous = false;
    const uint32_t bootAt = millis();
    uint32_t slotAt = bootAt + tdmaSlotMs(SLAVE_DEVICE_ID);
    uint32_t frame = 0, lastSyncAt = bootAt;
    uint32_t acceptedEpoch = 0, acceptedFrame = 0;
    for (;;)
    {
        SyncEvent event{};
        if (syncQueue && xQueueReceive(syncQueue, &event, 0) == pdPASS)
        {
            // A duplicate must neither extend SYNC validity nor repeat a TX.
            if (!haveSync || event.sync.epochMs != acceptedEpoch || event.sync.frame != acceptedFrame)
            {
                acceptedEpoch = event.sync.epochMs;
                acceptedFrame = event.sync.frame;
                slotAt = event.receivedAtMs + event.sync.remainingMs + tdmaSlotMs(SLAVE_DEVICE_ID);
                frame = event.sync.frame;
                lastSyncAt = event.receivedAtMs;
                haveSync = true;
                autonomous = false;
                Serial.printf("[TDMA] SYNC epoch=%lu frame=%lu slot=%lu mode=SYNCED\n",
                    static_cast<unsigned long>(acceptedEpoch), static_cast<unsigned long>(frame),
                    static_cast<unsigned long>(slotAt));
            }
        }
        const uint32_t now = millis();
        if (!autonomous && now - lastSyncAt >= TDMA_SYNC_TIMEOUT_MS)
        {
            autonomous = true;
            // Keep the existing phase; jitter must never shift later frames.
            Serial.printf("[TDMA] mode=AUTONOMOUS slave=%u (%s)\n", SLAVE_DEVICE_ID,
                          haveSync ? "SYNC lost" : "no SYNC since boot");
        }
        if ((!haveSync && !autonomous) || tdmaDelta(now, slotAt) < -150)
        {
            vTaskDelay(pdMS_TO_TICKS(5));
            continue;
        }
        const uint32_t target = slotAt;
        const uint16_t sequence = static_cast<uint16_t>(frame);
        slotAt += TDMA_FRAME_MS;
        ++frame; // One attempt per frame, even on CAD busy/error or TX failure.
        const uint32_t allowedLate = autonomous ? TDMA_AUTONOMOUS_JITTER_MS : TDMA_MAX_LATE_MS;
        if (tdmaDelta(now, target) > static_cast<int32_t>(allowedLate)) continue;
        const uint32_t jitter = autonomous ? esp_random() % (TDMA_AUTONOMOUS_JITTER_MS + 1) : 0;
        if (autonomous)
            Serial.printf("[TDMA] AUTO slave=%u frame=%u slot=%lu jitter=%lu\n",
                SLAVE_DEVICE_ID, sequence, static_cast<unsigned long>(target),
                static_cast<unsigned long>(jitter));

        GPS_packet latestGps{};
        const bool haveGps = gpsQueue && xQueuePeek(gpsQueue, &latestGps, 0) == pdTRUE;
        const bool gpsAvailable = haveGps && latestGps.updateId != lastSentGpsUpdateId &&
            millis() - latestGps.capturedAtMs <= GPS_MAX_AGE_MS;
        GPS_packet outgoingGps{};
        outgoingGps.hdop = UINT16_MAX;
        if (gpsAvailable) outgoingGps = latestGps;
        if (sendDogStatus(outgoingGps, sequence, gpsAvailable, target, autonomous, jitter) && gpsAvailable)
            lastSentGpsUpdateId = latestGps.updateId;
    }
}
