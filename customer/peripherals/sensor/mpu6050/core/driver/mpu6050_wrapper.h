/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MPU6050_WRAPPER_H__
#define __MPU6050_WRAPPER_H__

#include <rtthread.h>


#ifdef __cplusplus
extern "C" {
#endif

int sensor_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data);
int sensor_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data);
int sensor_get_ms(unsigned long *ms);
void sensor_mdelay(unsigned long ms);
int sensor_reg_int_cb(void (*cb)(void));

#ifdef __cplusplus
}
#endif
#endif /* __MPU6050_WRAPPER_H__ */

