/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INV_QUATERNION_SUPERVISOR_H__
#define INV_QUATERNION_SUPERVISOR_H__

#include "mltypes.h"

#ifdef __cplusplus
extern "C" {
#endif

inv_error_t inv_enable_quaternion(void);
inv_error_t inv_disable_quaternion(void);
inv_error_t inv_init_quaternion(void);
inv_error_t inv_start_quaternion(void);
void inv_set_quaternion(long *quat);

#ifdef __cplusplus
}
#endif

#endif // INV_QUATERNION_SUPERVISOR_H__
