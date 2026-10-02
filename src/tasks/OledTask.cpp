#include "OledTask.h"
#include "../core/DeviceIdentity.h"

QueueHandle_t oledQueue;

struct OledTransmission
{
    DogStatusPayload payload;
    uint16_t sequence;
    bool gpsAvailable;
    bool success;
};

static QueueHandle_t transmissionQueue = nullptr;

namespace {
constexpr uint8_t PRG_BUTTON_PIN = 0;
constexpr uint32_t OLED_ON_TIME_MS = 120000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;
constexpr uint32_t PAGE_SWITCH_MS = 2000;
bool oledPowered = false;
uint32_t oledActivatedMs = 0;
}

TrackerDisplay display;

static bool oledPowerOn()
{
    oledPowered = display.init();
    return oledPowered;
}

static void oledPowerOff()
{
    display.displayOff();
    oledPowered = false;
    Serial.println("[TFT] Backlight OFF; GNSS/TFT shared rail remains ON");
}

static void oledShowStartup()
{
    display.clear();
    char startupText[32];
    snprintf(startupText, sizeof(startupText), "Slave %u START",
             static_cast<unsigned>(SLAVE_DEVICE_ID));
    display.drawString(0, 0, startupText);
    display.display();
}

void oledInit(void)
{
    if (!TRACKER_TFT_ENABLED) return;
    transmissionQueue = xQueueCreate(1, sizeof(OledTransmission));
    if (transmissionQueue == nullptr)
        Serial.println("[TFT] Transmission queue CREATE FAIL");

    pinMode(PRG_BUTTON_PIN, INPUT_PULLUP);
    oledActivatedMs = millis();
    if (!oledPowerOn()) return;
    oledShowStartup();
    Serial.println("[TFT] Power ON (2 minute timer)");
}

void oledShowTransmission(const DogStatusPayload &payload, uint16_t sequence,
                          bool gpsAvailable, bool success)
{
    // Copy the exact transmitted data; only the OLED task accesses the display.
    const OledTransmission transmission{payload, sequence, gpsAvailable, success};
    if (transmissionQueue != nullptr)
        xQueueOverwrite(transmissionQueue, &transmission);
}

void oledTask(void *pv)
{
    (void)pv;
    OledTransmission transmission{};
    bool haveTransmission = false;
    uint8_t currentPage = 0;
    uint32_t lastPageMs = 0;
    int rawButtonState = HIGH;
    int stableButtonState = HIGH;
    uint32_t buttonChangedMs = millis();
    bool redrawPending = false;

    for (;;)
    {
        // Sample PRG independently of the slower display refresh.
        vTaskDelay(pdMS_TO_TICKS(10));
        const uint32_t now = millis();
        const int buttonState = digitalRead(PRG_BUTTON_PIN);
        if (buttonState != rawButtonState)
        {
            rawButtonState = buttonState;
            buttonChangedMs = now;
        }
        if (rawButtonState != stableButtonState &&
            (uint32_t)(now - buttonChangedMs) >= BUTTON_DEBOUNCE_MS)
        {
            stableButtonState = rawButtonState;
            // One event per press, including while the display is already on.
            if (stableButtonState == LOW)
            {
                oledActivatedMs = now;
                if (!oledPowered && oledPowerOn()) oledShowStartup();
                currentPage = 0;
                lastPageMs = now;
                redrawPending = true;
                Serial.println("[TFT] PRG pressed; restarted 2 minute timer");
            }
        }
        if (oledPowered && (uint32_t)(now - oledActivatedMs) >= OLED_ON_TIME_MS)
            oledPowerOff();

        // Keep the latest transmission while off without accessing the OLED.
        if (transmissionQueue != nullptr &&
            xQueueReceive(transmissionQueue, &transmission, 0) == pdPASS)
        {
            if (!haveTransmission)
                lastPageMs = now;
            haveTransmission = true;
            redrawPending = true;
        }

        if (oledPowered && haveTransmission)
        {
            const bool pageDue = (uint32_t)(now - lastPageMs) >= PAGE_SWITCH_MS;
            if (pageDue)
            {
                currentPage ^= 1;
                lastPageMs = now;
                redrawPending = true;
            }

            if (!redrawPending)
                continue;

            redrawPending = false;
            const DogStatusPayload &data = transmission.payload;
            char str[40];
            display.clear();

            if (currentPage == 0)
            {
                snprintf(str, sizeof(str), "S%u %s", data.slaveId,
                         transmission.success ? "OK" : "FAIL");
                display.drawString(0, 0, str);
                display.drawString(0, 12, transmission.gpsAvailable ? "GPS OK" : "GPS NA");
                if (data.activityValid)
                    snprintf(str, sizeof(str), "ACT %.1f", data.activityScore / 1000.0);
                else
                    snprintf(str, sizeof(str), "ACT NA");
                display.drawString(0, 24, str);
                if (data.batteryValid)
                    snprintf(str, sizeof(str), "BAT %u%%", data.batteryPercentage);
                else
                    snprintf(str, sizeof(str), "BAT NA");
                display.drawString(0, 36, str);
                display.drawString(0, 52, "P1");
            }
            else
            {
                if (transmission.gpsAvailable)
                {
                    snprintf(str, sizeof(str), "LAT %.5f", data.lat / 1000000.0);
                    display.drawString(0, 0, str);
                    snprintf(str, sizeof(str), "LON %.5f", data.lon / 1000000.0);
                    display.drawString(0, 12, str);
                    snprintf(str, sizeof(str), "SPD %.1f SAT %u",
                             data.speed / 100.0, data.satellites);
                    display.drawString(0, 24, str);
                    snprintf(str, sizeof(str), "UTC %06lu", static_cast<unsigned long>(data.gpsTimestamp));
                    display.drawString(0, 36, str);
                }
                else
                {
                    display.drawString(0, 0, "GPS NA");
                    display.drawString(0, 12, "No GPS data");
                    display.drawString(0, 24, "Wait for fix");
                }
                display.drawString(0, 52, "P2");
            }
            display.display();
        }
    }
}
