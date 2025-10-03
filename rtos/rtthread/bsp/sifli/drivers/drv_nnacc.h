/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_NNACC_H__
#define __DRV_NNACC_H__
#include "board.h"
#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_NNACC_MODULE_ENABLED
typedef void (*nn_acc_cbk)(void);

rt_err_t nn_acc_start(NNACC_ConfigTypeDef *config);
rt_err_t nn_acc_start_IT(NNACC_ConfigTypeDef *config, nn_acc_cbk cbk);
NNACC_HandleTypeDef *drv_get_nnacc_handle();
#endif /* HAL_NNACC_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif
#endif /*__DRV_CAN_H__ */

