/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_TI_AFE4404_H__
#define SENSOR_TI_AFE4404_H__


#include "board.h"
#include "sensor.h"

#include "pps960.h"



#ifdef __cplusplus
extern "C" {
#endif

/* afe4404 device structure */
struct afe4404_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
};

int rt_hw_afe4404_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_TI_AFE4404_H__
