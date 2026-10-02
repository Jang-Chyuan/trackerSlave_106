#include "LcdTask.h"
#include "../core/EventBus.h"
LiquidCrystal_PCF8574 lcd(0x27);

QueueHandle_t lcdQueue;
// QueueHandle_t loggerQueue;

void led1602_init()
{
    lcd.begin(16, 2); // 初始化LCD
    lcd.setBacklight(250);
    lcd.clear();
}

void lcdTask(void *pv)
{
    AppMessage msg;

    while (1)
    {
        if (xQueueReceive(lcdQueue, &msg, portMAX_DELAY))
        {
            lcd.clear();
            lcd.setCursor(0, 0); // 設定游標位置 (column,row)
            lcd.print(msg.topic);

            lcd.setCursor(0, 1); // 設定游標位置 (column,row)
            lcd.print(msg.payload);
            vTaskDelay(pdMS_TO_TICKS(900));
        }
    }
}
