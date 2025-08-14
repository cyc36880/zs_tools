#ifndef __POLLING_TASK_H__
#define __POLLING_TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../zst_conf.h"

#if (ZST_USE_PTASK == 1)
// #include "../container/include/cc_all.h"

/**
 * | tasks    
 * --------------------------
 * [ TASK ] --- TASK --- TASK
 *    |
 *    |
 *    task
 *    |     
 *    |       
 *    task 
 *          
 *    
 */


typedef struct _ptask_ ptask_t;

typedef void (*ptask_func_t)(ptask_t * ptask);

typedef enum
{
    PTASK_IDLE = 0,
    PTASK_RUNNING,
    PTASK_STOP,
} polling_task_state_t;

typedef struct
{
    ptask_func_t load;
    ptask_func_t run;
    ptask_func_t exit;
    void * param;
} ptask_base_t;

struct _ptask_
{
    ptask_t * prev;
    ptask_t * next;

    void * user_data;
    ptask_base_t base;
};

typedef struct _ptask_list_t
{
    ptask_t ** array;
    size_t size;
    
    ptask_t * current_task; // current task
    polling_task_state_t state;
    uint8_t auto_start : 1; // 不调用 ptask_start , 自动开启任务。默认开启
} ptask_list_t;

int ptask_init(ptask_list_t * ptask_list);
int ptask_run(ptask_list_t * ptask_list);

ptask_t * ptask_root_create(ptask_list_t * ptask_list, const ptask_base_t * ptask_base);
ptask_t * ptask_create(ptask_t *ptask_parent, const ptask_base_t * ptask_base);

// 不会清楚user_data，只是将任务释放
// void ptask_destroy(ptask_t * ptask);

// 会退出上一个任务，并切换到当前任务
int ptask_start(ptask_list_t * ptask_list, const ptask_t * ptask_root);

#ifdef __cplusplus
}
#endif

#endif /* ZST_USE_PTASK */

#endif /* __POLLING_TASK_H__ */
