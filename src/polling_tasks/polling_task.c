#include "polling_task.h"

#include "../misc/zst_mem.h"



int ptask_init(ptask_list_t *ptask_list)
{
    zst_memset_00(ptask_list, sizeof(ptask_list_t));
    return 0;
}


ptask_t * ptask_create(ptask_list_t *ptask_list, ptask_base_t *ptask_base)
{

    return 0;
}

ptask_t * ptask_sub_create(ptask_t *ptask_parent, ptask_base_t *ptask_base)
{

    return 0;
}
