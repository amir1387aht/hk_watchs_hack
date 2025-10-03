/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-07     aozima       the first version
 */

#ifndef __DRV_ENCODER_H_INCLUDE__
#define __DRV_ENCODER_H_INCLUDE__

#include <rtthread.h>
#include <rtdevice.h>


#define PULSE_ENCODER_CMD_ENABLE        (128 + 0)
#define PULSE_ENCODER_CMD_DISABLE       (128 + 1)
#define PULSE_ENCODER_CMD_GET_COUNT     (128 + 2)
#define PULSE_ENCODER_CMD_GET_SPEED     (128 + 3)
#define PULSE_ENCODER_CMD_SET_COUNT     (128 + 4)




struct rt_encoder_configuration
{
    rt_uint32_t channel;    /* Encoder channel */
    rt_int16_t get_count; /* time get_count */
    rt_int16_t set_count; /* time set_count */

    rt_uint16_t direction;
    rt_uint16_t speed;
};


struct rt_device_encoder;
struct rt_encoder_ops
{
    rt_err_t (*control)(struct rt_device_encoder *device, int cmd, void *arg);
};

struct rt_device_encoder
{
    struct rt_device parent;
    const struct rt_encoder_ops *ops;
};

/*ENABLE*/


rt_err_t rt_device_pulse_encoder_register(struct rt_device_encoder *device, const char *name, const struct rt_encoder_ops *ops, void *user_data);










#endif /* __DRV_ENCODER_H_INCLUDE__ */
