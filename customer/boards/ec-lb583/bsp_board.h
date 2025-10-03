/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BSP_BOARD_H__
#define __BSP_BOARD_H__

#include "rtconfig.h"
#include "drv_io.h"
#include "bf0_hal.h"
#ifdef PMIC_CTRL_ENABLE
    #include "pmic_controller.h"
#endif /* PMIC_CTRL_ENABLE */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __CC_ARM
#ifndef ROM_ATTR
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#else // ROM_ATTR
#define HEAP_BEGIN   ((void *)(LPSYS_RAM_START_ADDR+0x80000))
#endif // !ROM_ATTR
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#ifndef ROM_ATTR
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#else
#define HEAP_BEGIN   ((void *)(LPSYS_RAM_START_ADDR+0x80000))
#endif
#elif defined ( __GNUC__ )
extern int __bss_end;
#define HEAP_BEGIN      ((void *)&__bss_end)
#endif

#ifdef SOC_BF0_HCPU
#define HEAP_END       (HCPU_RAM_DATA_START_ADDR + HCPU_RAM_DATA_SIZE) //TODO:
#else
#ifndef ROM_ATTR
#define HEAP_END       (LCPU_RAM_DATA_START_ADDR + LCPU_RAM_DATA_SIZE) //TODO:
#else // ROM_ATTR
#define HEAP_END       (LCPU_ROM_RAM_START_ADDR)
#endif
#endif

void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */
