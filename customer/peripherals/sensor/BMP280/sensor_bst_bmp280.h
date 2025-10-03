/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_BST_BMP280_H__
#define SENSOR_BST_BMP280_H__


#include "board.h"
#include "sensor.h"

#include "BMP280.h"



#ifdef __cplusplus
extern "C" {
#endif

/* bmp280 config structure */
struct bmp280_config
{
    rt_uint16_t accel_range;
    rt_uint16_t gyro_range;
};

/* bmp280 device structure */
struct bmp280_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
    struct bmp280_config config;
};

int rt_hw_bmp280_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_BST_BMP280_H__
