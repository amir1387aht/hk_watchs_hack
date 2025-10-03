/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IPC_QUEUE_DEVICE_H
#define IPC_QUEUE_DEVICE_H
#include "rtthread.h"
#include <stdint.h>
#include <stdbool.h>
#include "ipc_queue.h"

/**
 ****************************************************************************************
* @addtogroup ipc_queue_device IPC Queue Wrapper Device
* @ingroup middleware
* @brief wrapper device interface for ipc_queue
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  register wrappter device for specified ipc queue
 *
 * @param[in] device rt device
 * @param[in] name device name
 * @param[in] queue ipc queue handle
 *
 * @retval error code
 */
rt_err_t ipc_queue_device_register(rt_device_t device, const char *name, ipc_queue_handle_t queue);

/**
 * @brief  rx notification callback for ipc queue which is bound with rt device
 *
 * The implemented notification callback would call rx_indicate callback of bound rt device
 *
 * @param[in] queue ipc queue handle
 * @param[in] size size in byte
 *
 * @retval status, 0: no error
 */
int32_t ipc_queue_device_rx_ind(ipc_queue_handle_t queue, size_t size);

/// @}  ipc_queue_device

#ifdef __cplusplus
}
#endif

/// @} file
#endif

