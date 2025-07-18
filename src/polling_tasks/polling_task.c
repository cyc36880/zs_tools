#include "polling_task.h"
#include <string.h>
#include "../misc/zst_mem.h"
// #include "stdio.h"
enum PTASK_STATE
{
    PTASK_STATE_LOAD,
    PTASK_STATE_RUN,
    PTASK_STATE_EXIT,
};


int ptask_init(ptask_list_t * ptask_list)
{
    zst_memset_00(ptask_list, sizeof(ptask_list_t));
    ptask_list->current_task_id = SIZE_MAX;
    return 0;
}


ptask_t * ptask_root_create(ptask_list_t * ptask_list, const ptask_base_t * ptask_base)
{
    ptask_t * ptask = zst_mem_alloc(sizeof(ptask_t));
    if (NULL == ptask)
    {
        return NULL;
    }
    zst_memset_00(ptask, sizeof(ptask_t));
    if (0 == ptask_list->size)
    {
        ptask_list->array = zst_mem_alloc(sizeof(ptask_t *));
        if (NULL == ptask_list->array)
        {
            zst_mem_free(ptask);
            return NULL;
        }
        ptask_list->array[0] = ptask;
        ptask_list->size = 1;
    }
    else
    {
        ptask_t ** ptask_array = zst_mem_realloc(ptask_list->array, sizeof(ptask_t *) * (ptask_list->size + 1));
        if (NULL == ptask_array)
        {
            zst_mem_free(ptask);
            return NULL;
        }
        ptask_list->array = ptask_array;
        ptask_list->array[ptask_list->size] = ptask;
        ptask_list->size++;
    }
    return NULL;
}

ptask_t * ptask_create(ptask_t * ptask_parent, const ptask_base_t * ptask_base)
{
    ptask_t * ptask = zst_mem_alloc(sizeof(ptask_t));
    if (NULL == ptask)
    {
        return NULL;
    }
    zst_memset_00(ptask, sizeof(ptask_t));
    memcpy(&ptask->base, ptask_base, sizeof(ptask_base_t));

    ptask_t * ptask_temp = ptask_parent;
    for ( ; NULL != ptask_temp->next; ptask_temp = ptask_temp->next)
    {
    }
    ptask->prev = ptask_temp;
    ptask_temp->next = ptask;

    return ptask;
}

// 递归加载
static void ptask_recursion_load(ptask_t * ptask_root)
{
    for (ptask_t * ptask = ptask_root; NULL != ptask; ptask = ptask->next)
    {
        if (ptask->base.load)
        {
            ptask->base.load(ptask);
        }
    }
}

// 递归运行
static void ptask_recursion_run(ptask_t * ptask_root)
{
    for (ptask_t * ptask = ptask_root; NULL != ptask; ptask = ptask->next)
    {
        // printf("ptask_recursion_run: %p\n", ptask);
        if (ptask->base.run)
        {
            ptask->base.run(ptask);
        }
    }
}

// 递归退出
static void ptask_recursion_exit(ptask_t * ptask_root)
{
    for (ptask_t * ptask = ptask_root; NULL != ptask; ptask = ptask->next)
    {
        if (ptask->base.exit)
        {
            ptask->base.exit(ptask);
        }
    }
}


static void ptask_recursion(ptask_t * ptask_root, enum PTASK_STATE state)
{
    if (NULL == ptask_root)
    {
        return;
    }
    switch (state)
    {
        case PTASK_STATE_LOAD:
            ptask_recursion_load(ptask_root);
            break;
        case PTASK_STATE_RUN:
            ptask_recursion_run(ptask_root);
            break;
        case PTASK_STATE_EXIT:
            ptask_recursion_exit(ptask_root);
            break;
        default:
            break;
    }
}

int ptask_run(ptask_list_t * ptask_list)
{
    if (NULL == ptask_list->array || ptask_list->current_task_id==SIZE_MAX)
    {
        return -1;
    }

    if (ptask_list->current_task_id >= ptask_list->size)
    {
        return -2;
    }

    ptask_recursion(ptask_list->array[ptask_list->current_task_id], PTASK_STATE_RUN);

    return 0;
}
