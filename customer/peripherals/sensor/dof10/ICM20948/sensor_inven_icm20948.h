/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_INVEN_ICM20948_H__
#define SENSOR_INVEN_ICM20948_H__


#include "ICM20948.h"


#ifdef __cplusplus
extern "C" {
#endif

/* define ICM-20948 Device I2C address*/
#define I2C_ADD_ICM20948_HIGH               0x69
#define I2C_ADD_ICM20948_LOW                0x68

#define ICM20948_ADDR_DEFAULT  I2C_ADD_ICM20948_LOW

/* Accelerometer full scale range */
enum icm20948_accel_range
{
    ICM20948_ACCEL_RANGE_2G  = 0, // ±2G
    ICM20948_ACCEL_RANGE_4G  = 1, // ±4G
    ICM20948_ACCEL_RANGE_8G  = 2, // ±8G
    ICM20948_ACCEL_RANGE_16G = 3  // ±16G
};

/* Gyroscope full scale range */
enum icm20948_gyro_range
{
    ICM20948_GYRO_RANGE_250DPS  = 0, // ±250°/s
    ICM20948_GYRO_RANGE_500DPS  = 1, // ±500°/s
    ICM20948_GYRO_RANGE_1000DPS = 2, // ±1000°/s
    ICM20948_GYRO_RANGE_2000DPS = 3  // ±2000°/s
};

/* Digital Low Pass Filter parameters */
enum icm20948_dlpf
{
    ICM20948_DLPF_DISABLE = 0, //256HZ
    ICM20948_DLPF_188HZ = 1,
    ICM20948_DLPF_98HZ  = 2,
    ICM20948_DLPF_42HZ  = 3,
    ICM20948_DLPF_20HZ  = 4,
    ICM20948_DLPF_10HZ  = 5,
    ICM20948_DLPF_5HZ   = 6
};

/* sleep mode parameters */
enum icm20948_sleep
{
    ICM20948_SLEEP_DISABLE = 0,
    ICM20948_SLEEP_ENABLE  = 1
};

/* Supported configuration items */
enum icm20948_cmd
{
    ICM20948_GYRO_RANGE,  /* Gyroscope full scale range */
    ICM20948_ACCEL_RANGE, /* Accelerometer full scale range */
    ICM20948_DLPF_CONFIG, /* Digital Low Pass Filter */
    ICM20948_SAMPLE_RATE, /* Sample Rate —— 16-bit unsigned value.
                            Sample Rate = [1000 -  4]HZ when dlpf is enable
                            Sample Rate = [8000 - 32]HZ when dlpf is disable */
    ICM20948_SLEEP        /* Sleep mode */
};

/* 3-axis data structure */
struct icm20948_3axes
{
    rt_int16_t x;
    rt_int16_t y;
    rt_int16_t z;
};

/* icm20948 config structure */
struct icm20948_config
{
    rt_uint16_t accel_range;
    rt_uint16_t gyro_range;
};

/* icm20948 device structure */
struct icm20948_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
    struct icm20948_config config;
};

int rt_hw_icm20948_init(const char *name, struct rt_sensor_config *cfg);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_INVEN_ICM20948_H__
