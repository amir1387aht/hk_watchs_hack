/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IPC_OS_PORT_H
#define IPC_OS_PORT_H
#include "rtthread.h"
#include <rthw.h>

#ifdef __cplusplus
extern "C" {
#endif

#define os_interrupt_disable()      rt_hw_interrupt_disable()

#define os_interrupt_enable(mask)   rt_hw_interrupt_enable(mask)

#define os_interrupt_enter()        rt_interrupt_enter()
#define os_interrupt_exit()         rt_interrupt_leave()

/// @}  file

#ifdef __cplusplus
}
#endif

/// @}
#endif

