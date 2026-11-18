#ifndef __ZST_SOFT_I2C_H__
#define __ZST_SOFT_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"
#if (ZST_SOFT_I2C == 1)

#define ZST_SOFT_I2C_W     0
#define ZST_SOFT_I2C_R     1 

#define ZST_SOFT_I2C_ACK   0
#define ZST_SOFT_I2C_NACK  1


typedef struct zst_soft_i2c zst_soft_i2c_t;


struct zst_soft_i2c
{
    void (*sda_input)(zst_soft_i2c_t * self);  // 对于开漏输出，可以忽略
    void (*sda_output)(zst_soft_i2c_t * self); // 对于开漏输出，可以忽略
    void (*delay)(zst_soft_i2c_t *self, uint16_t time_us);
    void (*scl_write)(zst_soft_i2c_t *self, uint8_t);
    void (*sda_write)(zst_soft_i2c_t *self, uint8_t);
    uint8_t (*sda_read)(zst_soft_i2c_t *self);

    uint16_t delay_time_us; // Phase delay: default 10 us, setter minimum 5 us.
    uint16_t waitack_delay_time_us; // Extra ACK polling time in us, default 10 us.

    void * user_data;
    /* Required: read the physical SCL input, not the output latch. */
    uint8_t (*scl_read)(zst_soft_i2c_t *self);
    uint16_t stretch_timeout_us; // Per rising edge; default 10000 us.
};

/* Single-master, 7-bit addresses (not shifted). External pull-ups required.
 * SDA/SCL outputs must be open-drain; write(1) means release, not drive high.
 * Optional SDA direction callbacks may be omitted for open-drain GPIO.
 * Initialize GPIO and the us timer before init(), which releases the bus.
 * scl_read is required. Clock stretching is supported; arbitration is not.
 * Transfers return -1 on NACK, stuck bus, SCL timeout or invalid arguments.
 * Use the delay setter (minimum 5 us), rather than writing the field directly.
 */
void zst_soft_i2c_init(zst_soft_i2c_t * self);

void zst_soft_i2c_set_delay_time_us(zst_soft_i2c_t * self, uint16_t delay_time_us);
void zst_soft_i2c_set_waitack_delay_time_us(zst_soft_i2c_t * self, uint16_t waitack_delay_time_us);

uint16_t zst_soft_i2c_get_delay_time_us(zst_soft_i2c_t * self);
uint16_t zst_soft_i2c_get_waitack_delay_time_us(zst_soft_i2c_t * self);


int zst_soft_i2c_is_device_ready(zst_soft_i2c_t * self, uint8_t device_address);
int zst_soft_i2c_write_bytes(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * data, uint16_t len);
int zst_soft_i2c_read_bytes(zst_soft_i2c_t * self,  uint8_t device_address, uint8_t * data, uint16_t len);
int zst_soft_i2c_write_double(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * reg_data, uint16_t reg_data_len, uint8_t * data, uint16_t data_len);
int zst_soft_i2c_read_double(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * reg_data, uint16_t reg_data_len, uint8_t * data, uint16_t data_len);
int zst_soft_i2c_write_reg(zst_soft_i2c_t * self, uint8_t device_address, uint8_t reg, uint8_t * data, uint16_t len);
int zst_soft_i2c_read_reg(zst_soft_i2c_t * self, uint8_t device_address, uint8_t reg, uint8_t * data, uint16_t len);

#endif
#ifdef __cplusplus
}
#endif

#endif /* __ZST_SOFT_I2C_H__ */
