#pragma once
#include <Arduino.h>
#include <vector>
#include <map>
#include "AppMessage.h"

class EventBus
{
public:
    void subscribe(const char* topicPrefix, QueueHandle_t q);
    void publish(const AppMessage& msg);

private:
    std::map<String, std::vector<QueueHandle_t>> subs;//map (key:topicPrefix, value:list of queues)
};

extern EventBus gBus;