#include "EventBus.h"

EventBus gBus;

void EventBus::subscribe(const char* topicPrefix, QueueHandle_t q)
{
    subs[String(topicPrefix)].push_back(q);
}

void EventBus::publish(const AppMessage& msg)
{
    String t = String(msg.topic);

    for (auto& entry : subs)
    {
        if (t.startsWith(entry.first))
        {
            for (auto& q : entry.second)
                xQueueSend(q, &msg, 0);
        }
    }
}