#ifndef __ZST_HAL_DELAY_H__
#define __ZST_HAL_DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"

typedef void (*hal_delay_func_t)(uint32_t delay_unit);

void zst_register_delay_us(hal_delay_func_t func);
void zst_register_delay_ms(hal_delay_func_t func);

void zst_delay_us(uint32_t delay_us);
void zst_delay_ms(uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* __ZST_HAL_DELAY_H__ */
