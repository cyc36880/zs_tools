#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "esp_task_wdt.h"

#include "zst_core.h"

static ptask_list_t task_list;
static ptask_t  * ptask1;
static ptask_t  * ptask2;

static ptask_t * ptask1_1;

void ptask_load(ptask_t * ptask)
{
    Serial.printf("ptask load %d\n", (uint32_t)ptask->user_data);
}

void ptask_func(ptask_t * ptask)
{
    const ptask_t * ptask_array[] = {ptask1, ptask2};
    static uint8_t count = 0;
    static uint8_t task_id = 0;
    Serial.printf("Hello World! %d\n", (uint32_t)ptask->user_data);
    if (++count == 10)
    {
        if (++task_id > 1)
            task_id = 0;
        count=0;
        ptask_start(&task_list, ptask_array[task_id]);
    }
}

void ptask_func_exit(ptask_t * ptask)
{
    Serial.printf("ptask_func_exit! %d\n", (uint32_t)ptask->user_data);
}


void setup()
{
    Serial.begin(115200);
    zst_init();
    ptask_init(&task_list);

    ptask_base_t ptask_base = {
        .load = ptask_load,
        .run = ptask_func,
    };
    // 创建任务根 1
    ptask1 = ptask_root_create(&task_list, &ptask_base);
    ptask1->user_data = (uint32_t *) 1;

    // 创建任务根 2
    ptask2 = ptask_root_create(&task_list, &ptask_base);
    ptask2->user_data = (uint32_t *) 2;

    // 创建任务 1.1
    ptask_base.exit = ptask_func_exit;
    ptask1_1 = ptask_create(ptask1, &ptask_base);
    ptask1_1->user_data = (uint32_t *) 3;

    // ptask_start(&task_list, ptask1); // 默认自动开启任务
}

void loop()
{
    ptask_run(&task_list);
    delay(1000);
}

