#pragma once

#include <Arduino.h>
#include <RadioLib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "LoRaPacket.h"



class LoRaDriver
{

public:

    bool begin();

    void loop();


    bool send(
        const LoRaPacket& pkt
    );

    bool sendScheduled(const LoRaPacket& pkt, uint32_t slotAtMs,
                       bool autonomous = false, uint32_t jitterMs = 0);

    void setFemLnaEnabled(bool enabled);

    bool isFemLnaEnabled() const;



private:


    bool sendImpl(const LoRaPacket& pkt, bool autonomous, uint32_t slotAtMs);
    bool autonomousChannelFree();
    static void onRxDone();


    static volatile bool rxFlag;
    static volatile uint32_t rxDoneMs;


    bool startReceive();



    Module* module=nullptr;


    SX1262* radio=nullptr;
    SemaphoreHandle_t radioMutex = nullptr;
    bool ready = false;



    uint16_t txSeq=0;

};



extern LoRaDriver LoRa;
