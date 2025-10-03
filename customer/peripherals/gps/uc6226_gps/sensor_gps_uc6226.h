/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_GPS_UC6226_H__
#define SENSOR_GPS_UC6226_H__


#include "um_gps_if.h"
#include "um_gps_hal.h"


#ifdef __cplusplus
extern "C" {
#endif

/* uc6226 config structure */
struct uc6226_config
{
    rt_uint32_t reserved;
};

/* uc6226 device structure */
struct uc6226_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t dev_addr;
    struct uc6226_config config;
};

int rt_hw_uc6226_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_GPS_UC6226_H__
