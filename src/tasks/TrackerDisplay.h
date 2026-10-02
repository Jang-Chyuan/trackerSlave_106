#pragma once
#include "HT_st7735.h"
#include "TrackerConfig.h"

// Compatibility facade: the old task name stays to preserve its callers.
// HT_st7735 uses HSPI; RadioLib uses the separate default FSPI bus.
class TrackerDisplay {
    HT_st7735 tft;
    bool initialized = false;
public:
    bool init() {
        if (!initialized) {
            tft.st7735_init();
            initialized = true;
        }
        digitalWrite(TRACKER_TFT_BACKLIGHT, HIGH);
        return true; // SPI display has no readback; verify the panel visually.
    }
    void clear() { tft.st7735_fill_screen(ST7735_BLACK); }
    void drawString(int x, int y, const char* text) {
        tft.st7735_write_str(x, y, text, Font_7x10, ST7735_WHITE, ST7735_BLACK);
    }
    void display() {} // TFT writes are immediate.
    void displayOff() { digitalWrite(TRACKER_TFT_BACKLIGHT, LOW); }
};
