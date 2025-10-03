/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_PSRAM_H__
#define __DRV_PSRAM_H__

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>
#include <drv_common.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*** define OPI psram base address for memory **/
#define PSRAM_BASE_ADDR             (PSRAM_BASE)

/* output api */
/**
 * @brief PSRAM hardware initial.
 * @param[none] .
 * @return 0 if initial success.
 */
int rt_psram_init(void);

/**
 * @brief Get PSRAM clock frequency.
 * @param addr PSRAM address
 * @return Clock freqency for PSRAM
 */
uint32_t rt_psram_get_clk(uint32_t addr);

/**
 * @brief PSRAM hardware enter low power.
 * @param name name of PSRAM controller.
 * @return RT_EOK if initial success, otherwise, -RT_ERROR.
 */
int rt_psram_enter_low_power(char *name);

int rt_psram_exit_low_power(char *name);

/**
 * @brief PSRAM set partial array self-refresh.
 * @param name name of PSRAM controller.
 * @param top set top part to self-refresh, else set bottom.
 * @param deno denomenator for refresh, like 2 for 1/2 to refresh, only support 2^n,
 *         when larger than 16, all memory not refresh, when 1 or 0, all meory auto refress by default.
 * @return 0 if success.
 */
int rt_psram_set_pasr(char *name, uint8_t top, uint8_t deno);

/**
 * @brief PSRAM auto calibrate delay.
 * @param name name of PSRAM controller.
 * @param sck sck delay pointer.
 * @param dqs dqs delay pointer
 * @return 0 if success.
 */
int rt_psram_auto_calib(char *name, uint8_t *sck, uint8_t *dqs);

#ifndef SF32LB55X
void rt_psram_wait_idle(char *name);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_PSRAM_H__ */

