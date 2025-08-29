#include "run_one.h"

#if ZST_RUN_ONE == 1

static uint64_t fnv1a_64(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = 0xcbf29ce484222325; // FNV offset basis for 64-bit

    for (size_t i = 0; i < len; i++)
    {
        hash ^= bytes[i];
        hash *= 0x100000001b3; // FNV prime for 64-bit
    }
    return hash;
}

int run_one_init(run_one_t * run_one_d, void * data, size_t size)
{
    run_one_d->data = data;
    run_one_d->size = size;
    run_one_d->last_hash = fnv1a_64(data, size);
    return 0;
}

/**
 * 返回0触发一次，否侧不触发
 */
int run_one_data_changed(run_one_t * run_one_d)
{
    uint64_t hash = fnv1a_64(run_one_d->data, run_one_d->size);
    if (hash != run_one_d->last_hash)
    {
        run_one_d->last_hash = hash;
        return 0;
    }
    return 1;
}


int run_one_data_equals(run_one_t * run_one_d, void * data, size_t size)
{
    uint64_t d_hash = fnv1a_64(run_one_d->data, run_one_d->size);
    uint64_t hash = fnv1a_64(data, size);
    if (d_hash == hash)
    {
        if (0 == run_one_d->last_hash)
        {
            run_one_d->last_hash = 1;
            return 0;
        }
    }
    else
    {
        run_one_d->last_hash = 0;
    }
    return 1;
}








#endif
