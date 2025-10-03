/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-08     balanceTWK   the first version
 */

#include <rtdevice.h>
#include <string.h>
#include <drivers/rt_drv_encoder.h>


/*ENABLE*/
static rt_err_t rt_pulse_encoder_open(struct rt_device *dev, rt_uint16_t oflag)
{
    struct rt_device_encoder *pulse_encoder;

    pulse_encoder = (struct rt_device_encoder *)dev;
    if (pulse_encoder->ops->control)
    {
        return pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_ENABLE, RT_NULL);
    }
    else
    {
        return -RT_ENOSYS;
    }
}


/*DISABLE*/
static rt_err_t rt_pulse_encoder_close(struct rt_device *dev)
{
    struct rt_device_encoder *pulse_encoder;

    pulse_encoder = (struct rt_device_encoder *)dev;
    if (pulse_encoder->ops->control)
    {
        return pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_DISABLE, RT_NULL);
    }
    else
    {
        return -RT_ENOSYS;
    }
}

/*
void *buffer: rt_uint16_t count
*/

static rt_size_t rt_pulse_encoder_read(struct rt_device *dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_err_t result = RT_EOK;

    struct rt_device_encoder *pulse_encoder;
    pulse_encoder = (struct rt_device_encoder *)dev;

    rt_int16_t *count = (rt_int16_t *)buffer;
    struct rt_encoder_configuration configuration = {0};

    if (pulse_encoder->ops->control)
    {
        result = pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_GET_COUNT, &configuration);
        if (result != RT_EOK)
        {
            return 0;
        }

        *count = configuration.get_count;
    }

    return size;
}

static rt_size_t rt_pulse_encoder_write(struct rt_device *dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    rt_err_t result = RT_EOK;

    struct rt_device_encoder *pulse_encoder;
    pulse_encoder = (struct rt_device_encoder *)dev;

    rt_int16_t *count = (rt_int16_t *)buffer;
    struct rt_encoder_configuration configuration = {0};

    if (pulse_encoder->ops->control)
    {
        configuration.set_count = *count;

        result = pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_SET_COUNT, &configuration);
        if (result != RT_EOK)
        {
            return 0;
        }


    }

    return size;
}

static rt_err_t rt_pulse_encoder_control(struct rt_device *dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    struct rt_device_encoder *encoder = (struct rt_device_encoder *)dev;

    if (encoder->ops->control)
    {
        result = encoder->ops->control(encoder, cmd, args);
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops pulse_encoder_ops =
{
    RT_NULL,
    rt_pulse_encoder_open;
    rt_pulse_encoder_close;
    rt_pulse_encoder_read;
    rt_pulse_encoder_write;
    rt_pulse_encoder_control
};
#endif

rt_err_t rt_device_pulse_encoder_register(struct rt_device_encoder *device, const char *name, const struct rt_encoder_ops *ops, void *user_data)
{

    if (device == RT_NULL)
    {
        rt_kprintf("error\n");
        return -RT_ERROR;
    }
    rt_err_t result = RT_EOK;
    memset(device, 0, sizeof(struct rt_device_encoder));

    device->parent.type        = RT_Device_Class_Miscellaneous;
    device->parent.rx_indicate = RT_NULL;
    device->parent.tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->parent.ops         = &pulse_encoder_ops;
#else
    device->parent.init        = RT_NULL;
    device->parent.open        = rt_pulse_encoder_open;
    device->parent.close       = rt_pulse_encoder_close;
    device->parent.read        = rt_pulse_encoder_read;
    device->parent.write       = rt_pulse_encoder_write;
    device->parent.control     = rt_pulse_encoder_control;
#endif
//  #endif

    device->ops = ops;
    device->parent.user_data   = user_data;

    result =  rt_device_register(&device->parent, name, RT_DEVICE_FLAG_RDWR);
    return result;
}





