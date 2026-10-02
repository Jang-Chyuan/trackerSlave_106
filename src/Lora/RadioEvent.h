#pragma once

#include "LoRaPacket.h"

typedef void (*PacketCallback)(const LoRaPacket&);

void RadioEvent_Begin(PacketCallback cb);

void RadioEvent_OnReceive(const LoRaPacket& pkt);