#include "LoRaDriver.h"
#include "TdmaProtocol.h"
#include "AppConfig.h"
#include "BoardConfig.h"
#include "FEM.h"
#include "tasks/Gps2LoraTask.h"

namespace
{
class RadioLock
{
public:
    explicit RadioLock(SemaphoreHandle_t mutex) : mutex_(mutex)
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
    }
    ~RadioLock() { xSemaphoreGive(mutex_); }
private:
    SemaphoreHandle_t mutex_;
};
}

LoRaDriver LoRa;
volatile bool LoRaDriver::rxFlag = false;
volatile uint32_t LoRaDriver::rxDoneMs = 0;

void LoRaDriver::onRxDone()
{
    rxDoneMs = millis();
    rxFlag = true;
}

bool LoRaDriver::begin()
{
    radioMutex = xSemaphoreCreateMutex();
    if (radioMutex == nullptr)
    {
        Serial.println("[LORA] Mutex CREATE FAIL");
        return false;
    }

    Serial.println("LoRa init");

    FEM.begin();

    SPI.begin(
        LORA_CLK,
        LORA_MISO,
        LORA_MOSI,
        RADIO_NSS);

    module =
        new Module(
            RADIO_NSS,
            RADIO_DIO1,
            RADIO_RESET,
            RADIO_BUSY);

    radio = new SX1262(module);

    int state =
        radio->begin(
            LORA_FREQ,
            LORA_BW,
            LORA_SF,
            LORA_CR,
            LORA_SYNC,
            LORA_RADIO_POWER,
            8, 1.8f); // preamble symbols, Tracker V2 TCXO voltage

    if (state != RADIOLIB_ERR_NONE)
    {

        Serial.print("LoRa FAIL=");
        Serial.println(state);

        return false;
    }

    Serial.println("SX1262 OK");

    state = radio->setCurrentLimit(LORA_CURRENT_LIMIT);

    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.print("LoRa OCP FAIL=");
        Serial.println(state);
        return false;
    }

    Serial.print("LoRa target power=");
    Serial.print(LORA_POWER);
    Serial.println(" dBm");

    Serial.print("SX1262 power=");
    Serial.print(LORA_RADIO_POWER);
    Serial.println(" dBm");

    Serial.print("SX1262 OCP=");
    Serial.print(LORA_CURRENT_LIMIT, 1);
    Serial.println(" mA");

    // Tracker V2 routes SX1262 DIO2 to the KCT8103L CPS pin.
    state = radio->setDio2AsRfSwitch(true);

    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.print("LoRa DIO2 RF SWITCH FAIL=");
        Serial.println(state);
        return false;
    }

    state = radio->setRxBoostedGainMode(
        LORA_SX1262_RX_BOOSTED_GAIN != 0);

    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.print("LoRa RX BOOST FAIL=");
        Serial.println(state);
        return false;
    }

    Serial.print("SX1262 RX boosted gain=");
    Serial.println(
        LORA_SX1262_RX_BOOSTED_GAIN ? "ON" : "OFF");

    Serial.print("KCT8103L RX LNA=");
    Serial.println(
        FEM.isRxLnaEnabled() ? "ON" : "BYPASS");

    state = radio->setCRC(true);

    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.print("LoRa CRC FAIL=");
        Serial.println(state);
        return false;
    }

    radio->setDio1Action(
        LoRaDriver::onRxDone);

    ready = startReceive();
    return ready;
}

bool LoRaDriver::startReceive()
{

    radio->standby();

    radio->clearIrqFlags(
        RADIOLIB_SX126X_IRQ_ALL);

    rxFlag = false;

    FEM.rxMode();
    radio->setDio1Action(LoRaDriver::onRxDone);

    int state =
        radio->startReceive();

    if (state != RADIOLIB_ERR_NONE)
    {

        Serial.print("RX FAIL=");
        Serial.println(state);

        return false;
    }

    Serial.println("RX READY");

    return true;
}

void LoRaDriver::loop()
{
    if (!ready) return;
    RadioLock lock(radioMutex);

    if (!rxFlag)
        return;

    const uint32_t receivedAtMs = rxDoneMs;
    rxFlag = false;
    if ((radio->getIrqFlags() & RADIOLIB_SX126X_IRQ_RX_DONE) == 0)
        return;

    uint8_t buffer[64];

    size_t len =radio->getPacketLength();

    if (len > 64)
        len = 64;

    int state =radio->readData(buffer,len);

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println("RX DATA");
        Serial.print("LEN=");
        Serial.println(len);
        Serial.print("RSSI=");
        Serial.println(radio->getRSSI());
        Serial.print("SNR=");
        Serial.println(radio->getSNR());

        if (len >= 4)
        {
            LoRaPacket packet{};
            packet.type = buffer[0];
            packet.seq = static_cast<uint16_t>(buffer[1]) |
                         (static_cast<uint16_t>(buffer[2]) << 8);
            packet.len = buffer[3];
            const size_t availablePayload = len - 4;
            if (packet.len <= LORA_MAX_PAYLOAD && packet.len <= availablePayload)
            {
                memcpy(packet.payload, &buffer[4], packet.len);
                gps2LoraHandlePacket(packet, receivedAtMs);
            }
            else
            {
                Serial.println("[LORA] Invalid payload length");
            }
        }
    }

    else
    {
        Serial.printf("[LORA] READ FAIL=%d\n", state);
    }

    startReceive();
}

bool LoRaDriver::send(const LoRaPacket &pkt)
{
    // All status traffic must use its TDMA slot.
    (void)pkt;
    return false;
}

bool LoRaDriver::sendScheduled(const LoRaPacket &pkt, uint32_t slotAtMs,
                               bool autonomous, uint32_t jitterMs)
{
    if (jitterMs > TDMA_AUTONOMOUS_JITTER_MS || (!autonomous && jitterMs != 0)) return false;
    const uint32_t target = slotAtMs + jitterMs;
    while (tdmaDelta(millis(), target) < 0)
    {
        // Cancel the old plan as soon as a new SYNC has been decoded.
        if (gps2LoraSyncPending()) return false;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return sendImpl(pkt, autonomous, slotAtMs);
}

// Called with radioMutex held and RX callback detached. Bound the wait rather
// than using blocking scanChannel(), which waits indefinitely for its IRQ.
bool LoRaDriver::autonomousChannelFree()
{
    FEM.rxMode();
    const int16_t state = radio->startChannelScan();
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.printf("[TDMA] CAD ERROR=%d; skip frame\n", state);
        return false;
    }
    const uint32_t startedAt = millis();
    while ((radio->getIrqFlags() & RADIOLIB_SX126X_IRQ_CAD_DONE) == 0)
    {
        if (millis() - startedAt >= TDMA_CAD_TIMEOUT_MS)
        {
            Serial.println("[TDMA] CAD TIMEOUT; skip frame");
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    const int16_t result = radio->getChannelScanResult();
    if (result != RADIOLIB_CHANNEL_FREE)
    {
        Serial.printf("[TDMA] CAD %s result=%d; skip frame\n",
                      result == RADIOLIB_LORA_DETECTED ? "BUSY" : "ERROR", result);
        return false;
    }
    return true;
}

bool LoRaDriver::sendImpl(const LoRaPacket &pkt, bool autonomous, uint32_t slotAtMs)
{
    if (!ready || pkt.len > LORA_MAX_PAYLOAD) return false;
    RadioLock lock(radioMutex);
    // Let loop() deliver pending RX (especially SYNC) instead of discarding it.
    if (rxFlag || gps2LoraSyncPending()) return false;

    const uint32_t airtimeMs = (radio->getTimeOnAir(pkt.len + 4) + 999) / 1000;
    const uint32_t windowMs = autonomous ? TDMA_AUTONOMOUS_WINDOW_MS : 1900;
    const int32_t initialLate = tdmaDelta(millis(), slotAtMs);
    if (initialLate < 0 || static_cast<uint32_t>(initialLate) + airtimeMs +
        (autonomous ? TDMA_CAD_TIMEOUT_MS : 0) > windowMs) return false;

    uint8_t buffer[64];
    buffer[0] = pkt.type;
    buffer[1] = pkt.seq & 0xff;
    buffer[2] = pkt.seq >> 8;
    buffer[3] = pkt.len;
    memcpy(&buffer[4], pkt.payload, pkt.len);

    // CAD_DONE and TX_DONE must not be interpreted as RX_DONE.
    radio->clearDio1Action();
    rxFlag = false;
    radio->standby();
    if (autonomous && !autonomousChannelFree())
    {
        startReceive();
        return false;
    }

    radio->standby();
    FEM.txMode();
    delay(2);

    // CAD and lock contention count against the original slot's budget.
    const int32_t late = tdmaDelta(millis(), slotAtMs);
    if (late < 0 || (!autonomous && late > static_cast<int32_t>(TDMA_MAX_LATE_MS)) ||
        static_cast<uint32_t>(late) + airtimeMs > windowMs)
    {
        Serial.println("[TDMA] TX window expired; skip frame");
        startReceive();
        return false;
    }
    const int state = radio->transmit(buffer, pkt.len + 4);
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.printf("TX ERROR=%d\n", state);
        startReceive();
        return false;
    }
    Serial.println("TX OK");
    return startReceive();
}

void LoRaDriver::setFemLnaEnabled(bool enabled)
{
    if (!ready) return;
    RadioLock lock(radioMutex);
    FEM.setRxLnaEnabled(enabled);

    if (radio != nullptr)
    {
        startReceive();
    }
}

bool LoRaDriver::isFemLnaEnabled() const
{
    return FEM.isRxLnaEnabled();
}
