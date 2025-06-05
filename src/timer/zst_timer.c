#include "zst_timer.h"
#include "../misc/zst_mem.h"
#include <stdlib.h>
#include <string.h>

#if (ZST_USE_TIMER == 1)

extern uint32_t zst_tick_get(void);
extern uint32_t zst_tick_elaps(uint32_t prev_tick);

static zst_timer_t *timer_list = NULL;
static bool timer_enabled = true;
static uint32_t last_idle_tick = 0;
static uint32_t idle_time_acc = 0;

void zst_timer_core_init(void)
{
    timer_list = NULL;
    timer_enabled = true;
    last_idle_tick = zst_tick_get();
    idle_time_acc = 0;
}

zst_timer_t *zst_timer_create_basic(void)
{
    zst_timer_t *timer = (zst_timer_t *)zst_mem_calloc(1, sizeof(zst_timer_t));
    if (!timer)
        return NULL;
    timer->repeat_count = -1; // 默认无限次
    timer->paused = false;
    timer->ready = false;
    timer->last_run = zst_tick_get();

    // 插入链表头
    timer->next = timer_list;
    timer_list = timer;
    return timer;
}

zst_timer_t *zst_timer_create(zst_timer_cb_t cb, uint32_t period, void *user_data)
{
    zst_timer_t *timer = zst_timer_create_basic();
    if (!timer)
        return NULL;
    timer->cb = cb;
    timer->period = period;
    timer->user_data = user_data;
    return timer;
}

void zst_timer_del(zst_timer_t *timer)
{
    zst_timer_t **curr = &timer_list;
    while (*curr)
    {
        if (*curr == timer)
        {
            *curr = timer->next;
            zst_mem_free(timer);
            return;
        }
        curr = &((*curr)->next);
    }
}

void zst_timer_pause(zst_timer_t *timer)
{
    if (timer)
        timer->paused = true;
}

void zst_timer_resume(zst_timer_t *timer)
{
    if (timer)
    {
        timer->paused = false;
        timer->last_run = zst_tick_get();
    }
}

void zst_timer_set_cb(zst_timer_t *timer, zst_timer_cb_t cb)
{
    if (timer)
        timer->cb = cb;
}

void zst_timer_set_period(zst_timer_t *timer, uint32_t period)
{
    if (timer)
        timer->period = period;
}

void zst_timer_set_repeat_count(zst_timer_t *timer, int32_t repeat_count)
{
    if (timer)
        timer->repeat_count = repeat_count;
}

void zst_timer_ready(zst_timer_t *timer)
{
    if (timer)
        timer->ready = true;
}

void * zst_timer_get_user_data(zst_timer_t *timer)
{
    if (timer)
        return timer->user_data;
    return NULL;
}

void zst_timer_reset(zst_timer_t *timer)
{
    if (timer)
        timer->last_run = zst_tick_get();
}

void zst_timer_enable(bool en)
{
    timer_enabled = en;
}

uint8_t zst_timer_get_idle(void)
{
    uint32_t now = zst_tick_get();
    uint32_t total = now - last_idle_tick;
    last_idle_tick = now;
    uint32_t idle = idle_time_acc;
    idle_time_acc = 0;
    if (total == 0)
        return 100;
    return (uint8_t)((idle * 100) / total);
}

zst_timer_t *zst_timer_get_next(zst_timer_t *timer)
{
    return timer ? timer->next : timer_list;
}

uint32_t zst_timer_handler(void)
{
    if (!timer_enabled)
        return 10;

    uint32_t now = zst_tick_get();
    uint32_t min_wait = 0xFFFFFFFF;
    bool idle = true;

    for (zst_timer_t *t = timer_list; t != NULL; t = t->next)
    {
        if (t->paused || !t->cb)
            continue;

        uint32_t elaps = zst_tick_elaps(t->last_run);
        if (t->ready || elaps >= t->period)
        {
            t->last_run = now;
            t->ready = false;

            if (t->repeat_count != 0)
            {
                t->cb(t);

                if (t->repeat_count > 0)
                {
                    t->repeat_count--;
                    if (t->repeat_count == 0)
                    {
                        t->paused = true;
                    }
                }
            }
            idle = false;
        }
        else
        {
            uint32_t wait = t->period - elaps;
            if (wait < min_wait)
                min_wait = wait;
        }
    }

    if (idle)
    {
        uint32_t idle_start = zst_tick_get();
        idle_time_acc += zst_tick_elaps(idle_start);
    }

    return min_wait;
}

uint32_t zst_timer_handler_run_in_period(uint32_t ms)
{
    static uint32_t last_tick = 0;
    uint32_t now = zst_tick_get();
    if (zst_tick_elaps(last_tick) >= ms)
    {
        last_tick = now;
        return zst_timer_handler();
    }
    return 1;
}

#endif // ZST_USE_TIMER
