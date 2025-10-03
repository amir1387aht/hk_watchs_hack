/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_AMS_TSL2572_H__
#define SENSOR_AMS_TSL2572_H__


#include "board.h"
#include "sensor.h"

#include "TSL2572.h"



#ifdef __cplusplus
extern "C" {
#endif

/* tsl2572 config structure */
struct tsl2572_config
{
    uint16_t int_thd_low;
    uint16_t int_thd_high;
};

/* tsl2572 device structure */
struct tsl2572_device
{
    rt_device_t bus;
    uint8_t id;
    uint8_t i2c_addr;
    struct tsl2572_config config;
};

int rt_hw_tsl2572_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_AMS_TSL2572_H__

