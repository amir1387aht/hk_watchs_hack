/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_GOODIX_GH3011_H__
#define SENSOR_GOODIX_GH3011_H__


#include "board.h"
#include "sensor.h"



#ifdef __cplusplus
extern "C" {
#endif

/* gh3011 device structure */
struct gh3011_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
};

int rt_hw_gh3011_register(const char *name, struct rt_sensor_config *cfg);
int rt_hw_gh3011_init(void);
int rt_hw_gh3011_deinit(void);


#ifdef __cplusplus
}
#endif
#endif  // SENSOR_GOODIX_GH3011_H__

