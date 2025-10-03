/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GSENSOR_SERVICE_H_
#define _GSENSOR_SERVICE_H_

#include "board.h"
#include "sensor.h"

#include "sc7a22.h"

#define GSENSOR_FIFO_SIZE                   (52 / 4)
#define SENSORS_ALGO_BUF_ARRAY              10      // odr 50hz, run 200ms interval

/* device structure */
struct sc7a22_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
};

#endif  // _GSENSOR_SERVICE_
