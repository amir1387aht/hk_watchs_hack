/*
 * SPDX-FileCopyrightText: 2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_LTR_303ALS_01_H__
#define SENSOR_LTR_303ALS_01_H__


#include "board.h"
#include "sensor.h"

#include "LTR303.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  struct rt_i2c_bus_device *bus;
} ltr303_device_t;

int rt_hw_ltr303_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_LTR_303ALS_01_H__

