#include "zst_hal_delay.h"

#include "zst_hal_tick.h"

static hal_delay_func_t delay_us_func= NULL;
static hal_delay_func_t delay_ms_func= NULL;




void zst_register_delay_us(hal_delay_func_t func)
{
    delay_us_func = func;
}

void zst_register_delay_ms(hal_delay_func_t func)
{
    delay_ms_func = func;
}



void zst_delay_us(uint32_t delay_us)
{
    if (delay_us_func != NULL)
    {
        delay_us_func(delay_us);
    }
}

void zst_delay_ms(uint32_t delay_ms)
{
    if (delay_ms_func != NULL)
    {
        delay_ms_func(delay_ms);
    }
    else if (delay_us_func != NULL)
    {
        while (delay_ms--) // 延时越长，误差越大
        {
            delay_us_func(1000);
        }
    }
    else
    {
        uint32_t last_tick = zst_tick_get();
        while (zst_tick_elaps(last_tick) < delay_ms);
    }
}

