/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_MAILBOX_H__
#define __DRV_MAILBOX_H__

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>

/** @brief hw mailbox init
*
* @details register configured hw mailbox device
*
* @return result #RT_EOK success, otherwise fail
*
*/
int rt_hw_mailbox_init(void);

#endif  /* __DRV_MAILBOX_H__ */

/** @} */  //drv_mailbox

