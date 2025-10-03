/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_STK8328C_H__
#define SENSOR_STK8328C_H__


#include "board.h"
#include "sensor.h"

#include "stk8328c.h"




#ifdef __cplusplus
extern "C" {
#endif

/* device structure */
struct stk8328c_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
};

int rt_hw_stk8328c_register(const char *name, struct rt_sensor_config *cfg);
int rt_hw_stk8328c_init(void);
int rt_hw_stk8328c_deinit(void);


#ifdef __cplusplus
}
#endif
#endif  // SENSOR_ST_SC7A22_H__
