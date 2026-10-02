#include "RadioEvent.h"

static PacketCallback callback = nullptr;

void RadioEvent_Begin(PacketCallback cb)
{
    callback = cb;
}

void RadioEvent_OnReceive(const LoRaPacket& pkt)
{
    if(callback)
        callback(pkt);
}