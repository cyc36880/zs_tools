#include "zst_soft_i2c.h"
#if (ZST_SOFT_I2C == 1)
static void pause_bus(zst_soft_i2c_t *s) { s->delay(s, s->delay_time_us < 5 ? 5 : s->delay_time_us); }
static void release_sda(zst_soft_i2c_t *s)
{
    s->sda_write(s, 1);
    if (s->sda_input)
        s->sda_input(s);
}
static void drive_sda(zst_soft_i2c_t *s, uint8_t v)
{
    s->sda_write(s, v);
    if (s->sda_output)
        s->sda_output(s);
}
/* Wait for the physical pin BEFORE timing the SCL high period. */
static int raise_scl(zst_soft_i2c_t *s)
{
    uint16_t elapsed = 0;
    s->scl_write(s, 1);
    while (!s->scl_read(s))
    {
        if (elapsed >= s->stretch_timeout_us)
            return -1;
        s->delay(s, 1);
        ++elapsed;
    }
    return 0;
}
static int valid_bus(zst_soft_i2c_t *s, uint8_t a)
{
    return s && s->delay && s->scl_write && s->scl_read && s->sda_write && s->sda_read && a <= 0x7F;
}
void zst_soft_i2c_init(zst_soft_i2c_t *s)
{
    if (!s)
        return;
    s->delay_time_us = 10;
    s->waitack_delay_time_us = 10;
    s->stretch_timeout_us = 10000;
    if (!valid_bus(s, 0))
        return;
    release_sda(s);
    if (!raise_scl(s))
        pause_bus(s);
}
static int start_bus(zst_soft_i2c_t *s, int repeated)
{
    if (repeated)
        s->scl_write(s, 0);
    release_sda(s);
    pause_bus(s);
    if (raise_scl(s))
        return -1;
    pause_bus(s);
    if (!s->sda_read(s))
        return -1;
    drive_sda(s, 0);
    pause_bus(s);
    s->scl_write(s, 0);
    pause_bus(s);
    return 0;
}
static int stop_bus(zst_soft_i2c_t *s)
{
    s->scl_write(s, 0);
    drive_sda(s, 0);
    pause_bus(s);
    if (raise_scl(s))
    {
        release_sda(s);
        return -1;
    }
    pause_bus(s);
    release_sda(s);
    pause_bus(s);
    return s->sda_read(s) ? 0 : -1;
}
static int write_byte(zst_soft_i2c_t *s, uint8_t v)
{
    uint16_t elapsed = 0;
    int nack;
    for (uint8_t i = 0; i < 8; ++i)
    {
        s->scl_write(s, 0);
        pause_bus(s);
        drive_sda(s, (v >> 7) & 1);
        v <<= 1;
        pause_bus(s);
        if (raise_scl(s))
            return -1;
        pause_bus(s);
        s->scl_write(s, 0);
    }
    pause_bus(s);
    release_sda(s);
    pause_bus(s);
    if (raise_scl(s))
        return -1;
    pause_bus(s);
    nack = s->sda_read(s) != 0;
    while (nack && elapsed < s->waitack_delay_time_us)
    {
        s->delay(s, 1);
        ++elapsed;
        nack = s->sda_read(s) != 0;
    }
    s->scl_write(s, 0);
    pause_bus(s);
    return nack ? -1 : 0;
}
static int read_byte(zst_soft_i2c_t *s, uint8_t *v, int last)
{
    uint8_t data = 0;
    s->scl_write(s, 0);
    release_sda(s);
    for (uint8_t i = 0; i < 8; ++i)
    {
        pause_bus(s);
        if (raise_scl(s))
            return -1;
        pause_bus(s);
        data = (uint8_t)((data << 1) | (s->sda_read(s) ? 1 : 0));
        s->scl_write(s, 0);
    }
    pause_bus(s);
    if (last)
        release_sda(s);
    else
        drive_sda(s, 0);
    pause_bus(s);
    if (raise_scl(s))
        return -1;
    pause_bus(s);
    s->scl_write(s, 0);
    pause_bus(s);
    release_sda(s);
    *v = data;
    return 0;
}
/* No retries: propagate every NACK/timeout, including failure to generate STOP. */
static int transfer(zst_soft_i2c_t *s, uint8_t a, uint8_t *reg, uint16_t rn, uint8_t *data, uint16_t n, int reading)
{
    int result = -1;
    if (!valid_bus(s, a))
        return -1;
    if (start_bus(s, 0))
    {
        release_sda(s);
        s->scl_write(s, 1);
        return -1;
    }
    if (write_byte(s, (uint8_t)((a << 1) | ((reading && !rn) ? 1 : 0))))
        goto done;
    for (uint16_t i = 0; i < rn; ++i)
        if (write_byte(s, reg[i]))
            goto done;
    if (reading && rn)
    {
        if (start_bus(s, 1) || write_byte(s, (uint8_t)((a << 1) | 1)))
            goto done;
    }
    for (uint16_t i = 0; i < n; ++i)
    {
        if (reading)
        {
            if (read_byte(s, &data[i], i == n - 1))
                goto done;
        }
        else if (write_byte(s, data[i]))
            goto done;
    }
    result = 0;
done:
    if (stop_bus(s))
        result = -1;
    return result;
}
void zst_soft_i2c_set_delay_time_us(zst_soft_i2c_t *s, uint16_t us) 
{ 
    s->delay_time_us = us < 5 ? 5 : us; 
}

void zst_soft_i2c_set_waitack_delay_time_us(zst_soft_i2c_t *s, uint16_t us) 
{ 
    s->waitack_delay_time_us = us; 
}

uint16_t zst_soft_i2c_get_delay_time_us(zst_soft_i2c_t *s) 
{ 
    return s->delay_time_us; 
}

uint16_t zst_soft_i2c_get_waitack_delay_time_us(zst_soft_i2c_t *s) 
{ 
    return s->waitack_delay_time_us; 
}

int zst_soft_i2c_is_device_ready(zst_soft_i2c_t *s, uint8_t a) 
{ 
    return transfer(s, a, NULL, 0, NULL, 0, 0); 
}

int zst_soft_i2c_write_bytes(zst_soft_i2c_t *s, uint8_t a, uint8_t *d, uint16_t n)
{
    if (!d || !n)
        return -1;
    return transfer(s, a, NULL, 0, d, n, 0);
}
int zst_soft_i2c_read_bytes(zst_soft_i2c_t *s, uint8_t a, uint8_t *d, uint16_t n)
{
    if (!d || !n)
        return -1;
    return transfer(s, a, NULL, 0, d, n, 1);
}
int zst_soft_i2c_write_double(zst_soft_i2c_t *s, uint8_t a, uint8_t *r, uint16_t rn, uint8_t *d, uint16_t n)
{
    if (!r || !rn || !d || !n)
        return -1;
    return transfer(s, a, r, rn, d, n, 0);
}
int zst_soft_i2c_read_double(zst_soft_i2c_t *s, uint8_t a, uint8_t *r, uint16_t rn, uint8_t *d, uint16_t n)
{
    if (!r || !rn || !d || !n)
        return -1;
    return transfer(s, a, r, rn, d, n, 1);
}
int zst_soft_i2c_write_reg(zst_soft_i2c_t *s, uint8_t a, uint8_t r, uint8_t *d, uint16_t n) 
{ 
    return zst_soft_i2c_write_double(s, a, &r, 1, d, n); 
}
int zst_soft_i2c_read_reg(zst_soft_i2c_t *s, uint8_t a, uint8_t r, uint8_t *d, uint16_t n) 
{ 
    return zst_soft_i2c_read_double(s, a, &r, 1, d, n); 
}
#endif
