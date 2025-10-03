/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "board.h"
#include "mpu6050_wrapper.h"

static struct rt_i2c_bus_device *sensor_i2c;

int sensor_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data)
{
    int ret = -1;
    rt_size_t ret_len;
    if (sensor_i2c)
    {
        ret_len =  rt_i2c_mem_write(sensor_i2c, slave_addr, reg_addr, 8, (void *)data, length);
        if (ret_len == length)
        {
            ret = 0;
        }
    }

    return ret;
}
int sensor_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data)
{
    int ret = -1;
    rt_size_t ret_len;
    if (sensor_i2c)
    {
        ret_len =  rt_i2c_mem_read(sensor_i2c, slave_addr, reg_addr, 8, data, length);
        if (ret_len == length)
        {
            ret = 0;
        }
    }

    return ret;
}

int sensor_get_ms(unsigned long *ms)
{
    rt_tick_t tick;

    tick = rt_tick_get();

    if (RT_TICK_PER_SECOND == 1000)
    {
        *ms = tick;
    }
    else
    {
        *ms = (uint64_t)tick * RT_TICK_PER_SECOND / 1000;
    }

    return 0;
}

void sensor_mdelay(unsigned long ms)
{
    rt_thread_mdelay(ms);
}

int sensor_reg_int_cb(void (*cb)(void))
{
    return 0;
}

int sensor_i2c_init(void)
{
    sensor_i2c = rt_i2c_bus_device_find(SENSOR_I2C);
    return RT_EOK;
}
INIT_BOARD_EXPORT(sensor_i2c_init);

/**
 *  @}
 */

