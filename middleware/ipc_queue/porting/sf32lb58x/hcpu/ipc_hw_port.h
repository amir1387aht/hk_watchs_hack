/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IPC_HW_PORT_H
#define IPC_HW_PORT_H
#include "register.h"

/**
 ****************************************************************************************
* @addtogroup ipc IPC Library
* @ingroup middleware
* @brief Sifli ipc library interface
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_HW_CH_NUM      (2)
#define IPC_HW_QUEUE_NUM   (8)

#define IPC_HL_HW_QUEUE_0  (0)
#define IPC_HL_HW_QUEUE_1  (1)
#define IPC_HL_HW_QUEUE_2  (2)
#define IPC_HL_HW_QUEUE_3  (3)
#define IPC_HL_HW_QUEUE_4  (4)
#define IPC_HL_HW_QUEUE_5  (5)
#define IPC_HL_HW_QUEUE_6  (6)
#define IPC_HL_HW_QUEUE_7  (7)

#define IPC_HA_HW_QUEUE_0  (IPC_HW_QUEUE_NUM + 0)
#define IPC_HA_HW_QUEUE_1  (IPC_HW_QUEUE_NUM + 1)
#define IPC_HA_HW_QUEUE_2  (IPC_HW_QUEUE_NUM + 2)
#define IPC_HA_HW_QUEUE_3  (IPC_HW_QUEUE_NUM + 3)
#define IPC_HA_HW_QUEUE_4  (IPC_HW_QUEUE_NUM + 4)
#define IPC_HA_HW_QUEUE_5  (IPC_HW_QUEUE_NUM + 5)
#define IPC_HA_HW_QUEUE_6  (IPC_HW_QUEUE_NUM + 6)
#define IPC_HA_HW_QUEUE_7  (IPC_HW_QUEUE_NUM + 7)

/// @}  file

#ifdef __cplusplus
}
#endif

/// @} button
#endif

