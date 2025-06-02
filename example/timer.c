#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "esp_task_wdt.h"

#include "../lib/zs_tools/zs_tool.h"


void zst_timer_cb(zst_timer_t *timer)
{
    zst_timer_del(timer);
    Serial.printf("timer_cb : %p\n", timer->user_data);
}



void setup()
{
    Serial.begin(115200);
    zst_init();
    zst_timer_t *timer = zst_timer_create(zst_timer_cb, 1000, NULL);
    zst_timer_create(zst_timer_cb, 1000, NULL);
    zst_timer_set_repeat_count(timer, 10);
    zst_timer_ready(timer);
}

void loop()
{
    zst_task_handler();
    delay(100);
}

