/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PPS960_HDR_FILE__
#define __PPS960_HDR_FILE__



#ifdef __cplusplus
extern "C" {
#endif

int init_pps960_sensor(void);
int open_pps960(void);
int close_pps960(void);

uint32_t pps960_get_hr(void);
uint32_t pps960_get_i2c_handle(void);

uint8_t pps960_get_dev_addr(void);

int pps960_self_check(void);

#ifdef __cplusplus
}
#endif
#endif  //__PPS960_HDR_FILE__

