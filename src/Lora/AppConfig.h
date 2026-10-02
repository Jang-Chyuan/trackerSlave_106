#pragma once


#define ROLE_MASTER    1
#define ROLE_SLAVE     2


//--------------------------------------------------
// Node Role
//--------------------------------------------------

 #define NODE_ROLE ROLE_SLAVE



//--------------------------------------------------
// LoRa SX1262
//--------------------------------------------------

#define LORA_FREQ      923.0

#define LORA_BW        125.0

#define LORA_SF        12

#define LORA_CR        5       // 4/5

#define LORA_SYNC      0x12

// Target power at the antenna after the onboard KCT8103L FEM.
#define LORA_POWER     20      // dBm

// Tracker V2 gain table: 20 dBm target - 14 dB gain = 6 dBm.
// Nominal only; verify conducted output on the actual board.
#define LORA_RADIO_POWER 6     // SX1262 output, dBm

// RadioLib defaults the SX1262 over-current protection to 60 mA.
// High-power TX needs the maximum value supported by RadioLib.
#define LORA_CURRENT_LIMIT 140.0f // mA

// Meshtastic defaults the Heltec V4.3 KCT8103L receive LNA to ON.
// Set to 0 only when strong local interference overloads the receiver.
#define LORA_FEM_LNA_ENABLED 1

// Meshtastic enables the SX1262 boosted RX gain mode for long-range RX.
#define LORA_SX1262_RX_BOOSTED_GAIN 1



//--------------------------------------------------
// Timing
//--------------------------------------------------

#define ACK_TIMEOUT        1500

#define HEARTBEAT_PERIOD   3000




