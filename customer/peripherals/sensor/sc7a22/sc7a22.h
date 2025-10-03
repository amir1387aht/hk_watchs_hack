/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SC7A22_H__
#define __SC7A22_H__

#include <stdint.h>
#include "sc7a22_reg.h"



#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SC7A22_ODR_DISABLE   =  0,
    SC7A22_ODR_1Hz6      =  1,
    SC7A22_ODR_12Hz5     =  2,
    SC7A22_ODR_25Hz      =  3,
    SC7A22_ODR_50Hz      =  4,
    SC7A22_ODR_100Hz     =  5,
    SC7A22_ODR_200Hz     =  6,
    SC7A22_ODR_400Hz     =  7,
    SC7A22_ODR_800Hz     =  8,
    SC7A22_ODR_1k6Hz     =  9,
    SC7A22_ODR_2k56Hz    = 10,
    SC7A22_ODR_4k257Hz   = 11,
} sc7a22_odr_t;

/*******************************************************************************/

int sc7a22_init(void);
uint32_t sc7a22_get_bus_handle(void);
uint8_t sc7a22_get_dev_addr(void);
uint8_t sc7a22_get_dev_id(void);
int sc7a22_open(void);
int sc7a22_close(void);

int sc7a22_set_fifo_mode(uint8_t val);
uint8_t sc7a22_get_fifo_count(void);
int sc7a22_read_fifo(uint8_t *buf, int len);
int sc7a20_read_fifo(uint8_t *buf, int len);
int sc7a22_set_fifo_threshold(int thd);
int sc7a22_self_check(void);

#ifdef __cplusplus
}
#endif
#endif /* __SC7A22_H__*/

