#ifndef __ZST_DHT11_H__
#define __ZST_DHT11_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"

#if ZST_DEVICE_DHT11 == 1

/**
 * 读取间隔在2s左右
 */

typedef struct _zst_dht11_
{
    void (*pin_mode)(struct _zst_dht11_ * self, uint8_t mode); // 0: input, 1: output
    void (*pin_write)(struct _zst_dht11_ * self, uint8_t value);
    uint8_t (*pin_read)(struct _zst_dht11_ * self);
    void * private_data;
} zst_dht11_t;


void zst_dht11_init(zst_dht11_t * handle);
int zst_dht11_read_original(zst_dht11_t * handle, uint8_t * temp_h, uint8_t * temp_l, uint8_t * hum_h, uint8_t * hum_l);
int zst_dht11_read(zst_dht11_t * handle, float * temperature, float * humidity);

#endif /* ZST_DEVICE_DHT11 == 1 */

#ifdef __cplusplus
}
#endif

#endif /* __ZST_DTH11_H__ */
