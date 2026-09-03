#include "zst_dht11.h"

#if ZST_DEVICE_DHT11 == 1
#include "../../hal/zst_hal.h"

static uint8_t read_byte(zst_dht11_t * handle);

void zst_dht11_init(zst_dht11_t * handle)
{
    if (NULL == handle || NULL == handle->pin_mode || NULL == handle->pin_write || NULL == handle->pin_read)
    {
        return;
    }
    handle->pin_mode(handle, 1);
    handle->pin_write(handle, 1);
}

int zst_dht11_read_original(zst_dht11_t * handle, uint8_t * temp_h, uint8_t * temp_l, uint8_t * hum_h, uint8_t * hum_l)
{
    if (NULL == handle || NULL == handle->pin_mode || NULL == handle->pin_write || NULL == handle->pin_read)
    {
        return -1;
    }

    uint8_t buffer[5] = {0};
    uint16_t wait_count = 0;

    handle->pin_mode(handle, 1); // 输出
    handle->pin_write(handle, 1);
    zst_delay_us(10);
    handle->pin_write(handle, 0);
    zst_delay_ms(20);
    handle->pin_write(handle, 1);
    zst_delay_us(30);
    handle->pin_mode(handle, 0); // 输入

    // 等待DHT11拉低响应（应在20-40us内），超时2ms
    wait_count = 0;
    while (handle->pin_read(handle) == 1)
    {
        if (++wait_count > 200) return -1;
        zst_delay_us(1);
    }

    // DHT11响应：80us低电平
    wait_count = 0;
    while (handle->pin_read(handle) == 0)
    {
        if (++wait_count > 200) return -1;
        zst_delay_us(1);
    }

    // DHT11响应：80us高电平
    wait_count = 0;
    while (handle->pin_read(handle) == 1)
    {
        if (++wait_count > 200) return -1;
        zst_delay_us(1);
    }

    // 读取40位数据（5字节）
    for (uint8_t index = 0; index < 5; index++)
    {
        buffer[index] = read_byte(handle);
        if (buffer[index] == 0xFF) return -1;
    }

    // 校验和验证
    if (buffer[4] != (buffer[0] + buffer[1] + buffer[2] + buffer[3]))
    {
        return -2;
    }

    // 解析数据
    if (temp_h) *temp_h = buffer[2];
    if (temp_l) *temp_l = buffer[3];
    if (hum_h) *hum_h = buffer[0];
    if (hum_l) *hum_l = buffer[1];

    return 0;
}

int zst_dht11_read(zst_dht11_t * handle, float * temperature, float * humidity)
{
    uint8_t temp_h = 0, temp_l = 0, hum_h = 0, hum_l = 0;
    if (0 != zst_dht11_read_original(handle, &temp_h, &temp_l, &hum_h, &hum_l))
    {
        return -1;
    }

    // 解析数据
    if (humidity) 
    {
        float fractional = hum_l;
        while (fractional > 1)
        {
            fractional /= 10;
        }
        *humidity = hum_h + fractional;
    }
    if (temperature) 
    {
        float temp = 0;
        float fractional = temp_l & 0x7F;
        while (fractional > 1)
        {
            fractional /= 10;
        }
        temp = temp_h + fractional;
        if (temp_l & 0x80) temp = -temp; // 判断是否为负数
        *temperature = temp;
    }
    
    return 0;
}

static uint8_t read_byte(zst_dht11_t * handle)
{
    uint8_t byte = 0;
    for (uint8_t loop=0; loop<8; loop++)
    {
        uint16_t wait_count = 0;
        // 等待引脚变高（50us低电平结束）
        while (handle->pin_read(handle) == 0)
        {
            if (++wait_count > 200) return 0xFF;
            zst_delay_us(1);
        }
        zst_delay_us(30); // 延时到数据脉冲中部采样
        if (handle->pin_read(handle) == 1) byte = (byte << 1) | 1; // 70us高电平=1
        else byte = (byte << 1) | 0; // 26us高电平=0
        // 等待引脚变低（数据脉冲结束），超时200us
        wait_count = 0;
        while (handle->pin_read(handle) == 1)
        {
            if (++wait_count > 200) return 0xFF;
            zst_delay_us(1);
        }
    }
    return byte;
}

#endif /* ZST_DEVICE_DHT11 == 1 */


