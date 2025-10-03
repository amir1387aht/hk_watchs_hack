/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IPC_OS_PORT_H
#define IPC_OS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#define os_interrupt_disable()        (0)

#define os_interrupt_enable(mask)

#define os_interrupt_enter()
#define os_interrupt_exit()

/// @}  file

#ifdef __cplusplus
}
#endif

/// @}
#endif

