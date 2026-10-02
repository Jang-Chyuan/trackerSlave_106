#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>


// ================= USB =================

#define USB_VID 0x303A
#define USB_PID 0x1001



// ================= Arduino GPIO =================

static const uint8_t LED_BUILTIN = 18;
#define BUILTIN_LED LED_BUILTIN


static const uint8_t TX = 43;
static const uint8_t RX = 44;



// ================= I2C =================

static const uint8_t SDA = 15;
static const uint8_t SCL = 16;



// ================= SPI =================

static const uint8_t SS   = 8;
static const uint8_t MOSI = 10;
static const uint8_t MISO = 11;
static const uint8_t SCK  = 9;



// Shared TFT/GNSS rail: HIGH = ON.
static const uint8_t Vext = 3;

// ================= LoRa SX1262 =================
// Heltec Wireless Tracker V2

// Radio GPIO macros belong to LoRa/BoardConfig.h and Heltec's board-config.h.
// Do not duplicate them as C constants: Heltec also includes this file from C.



// compatibility

static const uint8_t RST_LoRa  = 12;
static const uint8_t BUSY_LoRa = 13;
static const uint8_t DIO0      = 14;



// ================= Heltec LoRaWAN =================

#define HELTEC_BOARD 0

#define LoRaWAN_DEBUG_LEVEL 0

#define SLOW_CLK_TPYE 0



// ================= Analog =================

static const uint8_t BATTERY_PIN = 1;
static const uint8_t ADC_CTRL_PIN = 2;

static const uint8_t A0  = 1;
static const uint8_t A1  = 2;
static const uint8_t A2  = 3;
static const uint8_t A3  = 4;
static const uint8_t A4  = 5;
static const uint8_t A5  = 6;
static const uint8_t A6  = 7;
static const uint8_t A7  = 8;
static const uint8_t A8  = 9;
static const uint8_t A9  = 10;
static const uint8_t A10 = 11;
static const uint8_t A11 = 12;
static const uint8_t A12 = 13;
static const uint8_t A13 = 14;
static const uint8_t A14 = 15;
static const uint8_t A15 = 16;
static const uint8_t A16 = 17;
static const uint8_t A17 = 18;
static const uint8_t A18 = 19;
static const uint8_t A19 = 20;



#endif /* Pins_Arduino_h */
