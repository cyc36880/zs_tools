#ifndef __POLLING_TASK_H__
#define __POLLING_TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../zst_conf.h"

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
    
    size_t current_task_id; // current task
    // ptask_t *last_current_task;
} ptask_list_t;

int ptask_init(ptask_list_t * ptask_list);
int ptask_run(ptask_list_t * ptask_list);

ptask_t * ptask_root_create(ptask_list_t * ptask_list, const ptask_base_t * ptask_base);
ptask_t * ptask_create(ptask_t *ptask_parent, const ptask_base_t * ptask_base);

#ifdef __cplusplus
}
#endif

#endif /* __POLLING_TASK_H__ */
