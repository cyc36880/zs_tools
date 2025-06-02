#include "zst_mem.h"
#include "../../../zst_conf.h"
#if ZST_USE_ALLOC
    #include ZST_MEM_CUSTOM_INCLUDE
#endif




void zst_memset(void * dst, uint8_t v, size_t len)
{
    memset(dst, v, len);
}


void zst_memset_00(void * dst, size_t len)
{
    memset(dst, 0, len);
}


void * zst_mem_alloc(size_t size)
{
    #if ZST_USE_ALLOC
        return ZST_MEM_CUSTOM_ALLOC(size);
    #else
        return NULL;
    #endif
}
void * zst_mem_calloc(size_t nmemb, size_t size)
{
    #if ZST_USE_ALLOC
        void * ptr = zst_mem_alloc(nmemb * size);
        if (ptr)
            zst_memset_00(ptr, nmemb * size);
        return ptr;
    #else
        return NULL;
    #endif
}

void zst_mem_free(void * ptr)
{
    #if ZST_USE_ALLOC
        ZST_MEM_CUSTOM_FREE(ptr);
    #endif
}

void * zst_mem_realloc(void * ptr, size_t size)
{
    #if ZST_USE_ALLOC
        return ZST_MEM_CUSTOM_REALLOC(ptr, size);
    #else
        return NULL;
    #endif
}


