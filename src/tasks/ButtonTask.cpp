#include "ButtonTask.h"
#include "tasks/topics.h"
//int bt_onff13 = 19;
//int bt_up13 = 18;
//int bt_down13 = 17;
//int bt_start13 = 16;
int relay13 = 35;            // relay13 MAIN control lora pin35 LED PIN
int relay13TempState = HIGH; // the current state of relay13
int relay13State;

//int lastOnff13State;    // the previous state of button
//int currentOnff13State; // the current state of button

//int lastUp13State;
//int currentUp13State;

//int lastDown13State;
//int currentDown13State;

//int lastStart13State;
//int currentStart13State;

char State13_out[10];

int changeDown13_min = 0;
AppMessage msg;
QueueHandle_t buttonQueue;
QueueHandle_t remoteQueue;

void mqttCmd()
{
    if (xQueueReceive(remoteQueue, &msg, pdMS_TO_TICKS(100)))
    {
        if (strcmp(msg.topic, Topics::SET_RELAY_VALVE) == 0)
        {
            if (strcmp(msg.payload, "on") == 0)
            {
                digitalWrite(relay13, LOW); // relay13 LOW active
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            else if (strcmp(msg.payload, "off") == 0)
            {
                digitalWrite(relay13, HIGH); // relay13 LOW active
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
        else if (strcmp(msg.topic, Topics::RELAY_VALVE_TIMER) == 0)
        {
            digitalWrite(relay13, LOW); // relay13 LOW active
            int timerVal = atoi(msg.payload);
            changeDown13_min = timerVal;
            snprintf(msg.topic, sizeof(msg.topic), "countDown/timer");
            snprintf(msg.payload, sizeof(msg.payload), "%d", changeDown13_min);
            gBus.publish(msg);
        }
    }
}

void busCmd()
{
    if (xQueueReceive(buttonQueue, &msg, pdMS_TO_TICKS(100)))
    {
        if (strcmp(msg.topic, "relay/tDownCompleted") == 0)
        {
            if (strcmp(msg.payload, "tDown13Completed") == 0)
            {
                digitalWrite(relay13, HIGH);
                vTaskDelay(pdMS_TO_TICKS(50));

                relay13State = digitalRead(relay13);

                // AppMessage tx;
                snprintf(msg.topic, sizeof(msg.topic), "relay_valve/read13");
                snprintf(msg.payload, sizeof(msg.payload), "%s", relay13State ? "HIGH" : "LOW");
                gBus.publish(msg);
                vTaskDelay(pdMS_TO_TICKS(200));

                // snprintf(msg.topic, sizeof(msg.topic), Topics::RELAY_VALVE_STATUS);
                snprintf(msg.topic, sizeof(msg.topic), "%s", Topics::RELAY_VALVE_STATUS);

                snprintf(msg.payload, sizeof(msg.payload), "%d", relay13State);
                gBus.publish(msg);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
    }
}

void button_ini()
{
   // pinMode(bt_onff13, INPUT_PULLUP);  // switch on/off button
  //  pinMode(bt_up13, INPUT_PULLUP);    // switch bt_up button
  //  pinMode(bt_down13, INPUT_PULLUP);  // switch bt_down button
  //  pinMode(bt_start13, INPUT_PULLUP); // switch bt_start button
    pinMode(relay13, OUTPUT);          // relay13 MAIN control
    digitalWrite(relay13, HIGH);       // relay13 LOW active
};

void ReadRelay13State()
{
    relay13State = digitalRead(relay13);
    // snprintf(msg.topic, sizeof(msg.topic), "NGarden/relay_valve/status");
    snprintf(msg.topic, sizeof(msg.topic), "%s", Topics::RELAY_VALVE_STATUS);
    snprintf(msg.payload, sizeof(msg.payload), "%d", relay13State);
    gBus.publish(msg);
    vTaskDelay(pdMS_TO_TICKS(100));
}


void ButtonTask(void *pv)
{

    while (1)
    {
        ReadRelay13State();
        mqttCmd();
        busCmd();
      
        vTaskDelay(pdMS_TO_TICKS(100)); // ⭐ ;
    }
}