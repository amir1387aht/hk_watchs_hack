/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __STK8328C_H__
#define __STK8328C_H__

#include <stdint.h>
#include "stk8328c_reg.h"



/*******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************/
int stk8328c_init(void);
uint32_t stk8328c_get_bus_handle(void);
uint8_t stk8328c_get_dev_addr(void);
uint8_t stk8328c_get_dev_id(void);
int stk8328c_open(void);
int stk8328c_close(void);

int stk8328c_set_fifo_mode(uint8_t val);
uint8_t stk8328c_get_fifo_count(void);
int stk8328c_read_fifo(uint8_t *buf, int len);
//int sc7a22_set_fifo_threshold(int thd);
int stk8328c_self_check(void);

#ifdef __cplusplus
}
#endif
#endif /* __SC7A22_H__*/

