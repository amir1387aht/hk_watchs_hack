/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_MEMSIC_MMC36X0JK_H__
#define SENSOR_MEMSIC_MMC36X0JK_H__


#include "board.h"
#include "sensor.h"

#include "mmc36x0kj.h"



#ifdef __cplusplus
extern "C" {
#endif

/* mmc36x0kj device structure */
struct mmc36x0kj_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
};

int rt_hw_mmc36x0kj_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_MEMSIC_MMC36X0JK_H__
