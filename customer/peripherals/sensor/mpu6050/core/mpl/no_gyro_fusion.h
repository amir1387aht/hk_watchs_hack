/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MLDMP_NOGYROFUSION_H__
#define MLDMP_NOGYROFUSION_H__
#include "mltypes.h"

#ifdef __cplusplus
extern "C" {
#endif

inv_error_t inv_enable_no_gyro_fusion(void);
inv_error_t inv_disable_no_gyro_fusion(void);
inv_error_t inv_start_no_gyro_fusion(void);
inv_error_t inv_start_no_gyro_fusion(void);
inv_error_t inv_init_no_gyro_fusion(void);

#ifdef __cplusplus
}
#endif

#endif // MLDMP_NOGYROFUSION_H__
