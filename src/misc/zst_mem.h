#ifndef __ZST_MEM_H__
#define __ZST_MEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../zst_conf.h"

/**
 * Same as `memset` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param v value to set [0..255]
 * @param len number of byte to set
 */
void zst_memset(void * dst, uint8_t v, size_t len);

/**
 * Same as `memset(dst, 0x00, len)` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param len number of byte to set
 */
void zst_memset_00(void * dst, size_t len);

void * zst_mem_alloc(size_t size);
void * zst_mem_calloc(size_t nmemb, size_t size);
void zst_mem_free(void * ptr);
void * zst_mem_realloc(void * ptr, size_t size);

/**
 * when `ZST_USE_ALLOC==0` or `ZST_MEM_CUSTOM==1`, will return 0
 */
size_t zst_mem_free_size(void);
size_t zst_mem_used_size(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __UDC_MEM_H__ */
