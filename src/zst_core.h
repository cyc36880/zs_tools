#ifndef __ZST_CORE_H__
#define __ZST_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif    

#include "../../zst_conf.h"
#include "../zs_tool.h"

void zst_init(void);
void zst_task_handler(void);

#if (ZST_USE_TIMER == 1)
    extern ztimer_t zst_ztimer;
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ZST_CORE_H__ */
