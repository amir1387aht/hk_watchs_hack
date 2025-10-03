/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_WDT_H__
#define __DRV_WDT_H__

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>
#include <drv_common.h>

/**
 * @brief Register watchdog device
 *
 * @retval RT_EOK: success, others: fail
 */
#ifdef BSP_USING_WDT
    int rt_wdt_init(void);
#else
    #define rt_wdt_init() 0
#endif

#ifndef BSP_WDT_TIMEOUT
    #define BSP_WDT_TIMEOUT 10
#endif

#ifndef WDT_REBOOT_TIMEOUT
    #ifdef SOC_BF0_LCPU
        #define WDT_REBOOT_TIMEOUT   60
    #else
        #define WDT_REBOOT_TIMEOUT   50
    #endif
#endif

#ifndef IWDT_SLEEP_TIMEOUT
    #define IWDT_SLEEP_TIMEOUT 300
#endif

#define IWDT_RELOAD_DIFFTIME 5   // iwdt to wdt reload difference time

/**
 * @brief Restore watchdog configuration if it's lost after standby sleep
 *
 * @return void
 */
void rt_wdt_restore(void);

#endif  /* __DRV_WDT_H__ */

