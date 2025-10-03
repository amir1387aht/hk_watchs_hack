/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_VIRT_STEP_H__
#define SENSOR_VIRT_STEP_H__


#include "board.h"
#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* virt_step config structure */
struct virt_step_config
{
    rt_uint16_t odr;
    rt_uint16_t weight;
    rt_uint16_t height;
};

/* virt_step device structure */
struct virt_step_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
    struct virt_step_config config;
};

int rt_virt_step_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_VIRT_STEP_H__

