#include "zst_core.h"
#include "../../zst_conf.h"
#include "../zs_tool.h"

void zst_init(void)
{
    #if (ZST_USE_TIMER == 1)
        zst_timer_core_init();
    #endif
}



void zst_task_handler(void)
{
    #if (ZST_USE_TIMER == 1)
        zst_timer_handler();
    #endif
}



