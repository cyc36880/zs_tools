#ifndef __CC_CONF_H__
#define __CC_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../misc/zst_mem.h"


#define CC_MEM_ALLOC(x) zst_mem_alloc(x)
#define CC_MEM_FREE(x)  zst_mem_free(x)


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __CC_CONF_H__ */
