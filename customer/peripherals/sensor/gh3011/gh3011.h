/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef GOODIX_GH3011_H__
#define GOODIX_GH3011_H__


#include "board.h"
#include "sensor.h"
#include "gh3011_example.h"


#ifdef __cplusplus
extern "C" {
#endif

uint32_t gh3011_get_i2c_handle(void);

uint8_t gh3011_get_dev_addr(void);

int gh3011_self_check(void);

uint32_t gh3011_get_hr(void);

int init_gh3011_sensor(void);

int open_gh3011(void);

int close_gh3011(void);


#ifdef __cplusplus
}
#endif
#endif  // GOODIX_GH3011_H__

