
#include "CountDownTimer.h"
#include "tasks/topics.h"

int get13_sec;
int get13_min;

int set13_sec = 0;
int set13_min = 0;
int set13_hur = 0;

QueueHandle_t timerQueue;
QueueHandle_t relay13Queue;

Countimer tDown13;

void tDown13Complete()
{
    AppMessage msg1;
    snprintf(msg1.topic, sizeof(msg1.topic), "relay/tDownCompleted");
    snprintf(msg1.payload, sizeof(msg1.payload), "tDown13Completed");
    gBus.publish(msg1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

bool valveRunning = false;

void print13_time()
{
    AppMessage msg2;
    snprintf(msg2.topic, sizeof(msg2.topic), "timer_valve/status");
    snprintf(msg2.payload, sizeof(msg2.payload), "%s", tDown13.getCurrentTime());
    gBus.publish(msg2);

    //Serial.print(tDown13.getCurrentTime());

    vTaskDelay(pdMS_TO_TICKS(100));

    // snprintf(msg2.topic, sizeof(msg2.topic), "NGarden/timer_valve/status");
    snprintf(msg2.topic, sizeof(msg2.topic), "%s", Topics::TIMER_VALVE_STATUS);

    snprintf(msg2.payload, sizeof(msg2.payload), "%s", tDown13.getCurrentTime());
    gBus.publish(msg2);
    vTaskDelay(pdMS_TO_TICKS(200));
    /*
        snprintf(msg.topic, sizeof(msg.topic), "NGarden/relay_valve/status");
        snprintf(msg.payload, sizeof(msg.payload), "%s", "on");
        gBus.publish(msg);
        vTaskDelay(pdMS_TO_TICKS(200));*/
}

void CountDownTimerTask(void *pv)
{
    AppMessage msg3;

    tDown13.setInterval(print13_time, 1000);

    while (1)
    {
        if (xQueueReceive(timerQueue, &msg3, 0))
        {
            if (strcmp(msg3.topic, "countDown/timer") == 0)
            {
                long sec = atol(msg3.payload) * 60;
                int h = sec / 3600;
                int m = (sec % 3600) / 60;
                int s = sec % 60;

                tDown13.setCounter(h, m, s,
                                   tDown13.COUNT_DOWN,
                                   tDown13Complete);

                tDown13.start();
            }
        }

        tDown13.run();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
