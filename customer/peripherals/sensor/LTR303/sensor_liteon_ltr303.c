/*
 * SPDX-FileCopyrightText: 2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sensor_liteon_ltr303.h"

#ifdef RT_USING_SENSOR

#define DBG_TAG "sensor.ltr-303"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rt_size_t ltr303_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    // ltr303_device_t hdev = sensor->parent.user_data;
    struct rt_sensor_data *data = (struct rt_sensor_data *)buf;

    if (sensor->info.type == RT_SENSOR_CLASS_LIGHT)
    {
        uint16_t light_value;
        light_value = LTR303_ReadVisible();

        data->type = RT_SENSOR_CLASS_LIGHT;
        data->data.light = (rt_int32_t)(light_value);
        data->timestamp = rt_sensor_get_ts();
    }

    return 1;
}

rt_err_t ltr303_set_power(ltr303_device_t hdev, rt_uint8_t power)
{
    (void)hdev;
    if (power == RT_SENSOR_POWER_NORMAL)
    {
        LTR303_PowerOn();
    }
    else if (power == RT_SENSOR_POWER_DOWN)
    {
        LTR303_PowerOff();
    }
    else
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t ltr303_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    // ltr303_device_t hdev = sensor->parent.user_data;
    ltr303_device_t hdev;
    switch (cmd)
    {
        case RT_SENSOR_CTRL_SET_POWER:
        {
            result = ltr303_set_power(hdev, (rt_uint32_t)args & 0xff);
            break;
        }
        case RT_SENSOR_CTRL_SELF_TEST:
        {
            result =  -RT_EINVAL;
            break;
        }
        default:
        {
            result = RT_EOK;
            break;
        }
    }

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    ltr303_fetch_data,
    ltr303_control
};

int rt_hw_ltr303_init(const char *name, struct rt_sensor_config *cfg)
{
    int result = -RT_ERROR;
    rt_sensor_t sensor = RT_NULL;
    LTR303_Init(cfg);

    sensor = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (RT_NULL == sensor)
    {
        LOG_E("calloc failed");
        return -RT_ERROR;
    }

    sensor->info.type       = RT_SENSOR_CLASS_LIGHT;
    sensor->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor->info.model      = "ltr303_light";
    sensor->info.unit       = RT_SENSOR_UNIT_LUX;
    sensor->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor->info.range_max  = 65535;
    sensor->info.range_min  = 1;
    sensor->info.period_min = 120;

    rt_memcpy(&sensor->config, cfg, sizeof(struct rt_sensor_config));
    sensor->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor, name, RT_DEVICE_FLAG_RDWR, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        rt_free(sensor);
        return -RT_ERROR;
    }
    else
    {
        LOG_I("light sensor init success");
        return RT_EOK;
    }
}

#endif // RT_USING_SENSOR

