/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdlib.h>
#include "utest.h"
#include "board.h"
#include "bf0_hal.h"
#include "tc_utils.h"

#ifdef HAL_LPTIM_MODULE_ENABLED
/*
    This example demo:
        1. Using LPTIM1 (hwp_lptim1) to generate a one-time timeout interrupt in 3.5 seconds
*/

static LPTIM_HandleTypeDef LPTIM_Handle = {0};

static HAL_StatusTypeDef lp_timer_init()
{
    HAL_StatusTypeDef r = HAL_OK;

    LPTIM_Handle.Instance = hwp_lptim1;
    LPTIM_Handle.Mode = HAL_LPTIM_ONESHOT;
    LPTIM_Handle.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV2; // Set prescaler divider, freqency is LXT_FREQ/2
    if (HAL_LPTIM_Init(&LPTIM_Handle) != HAL_OK)
        r = HAL_ERROR;
    else
    {
        HAL_NVIC_SetPriority(LPTIM1_IRQn, 3, 0);    /* set the TIMx priority */
        HAL_NVIC_EnableIRQ(LPTIM1_IRQn);            /* enable the TIMx global Interrupt */
    }

    return r;
}

void LPTIM1_IRQHandler(void)
{
    ENTER_INTERRUPT();
    LOG_I("LPTIM1 timeout");
    HAL_LPTIM_IRQHandler(&LPTIM_Handle);
    LEAVE_INTERRUPT();
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(int argc, char **argv)
{
    lp_timer_init();
    LOG_I("Start LPTIMER for 3.5 seconds");

    HAL_LPTIM_Counter_Start_IT(&LPTIM_Handle, 3500 * (HAL_LPTIM_GetFreq() / 2) / 1000); // Set counter, timeout in 3.5seconds
    rt_thread_mdelay(4000);
}

UTEST_TC_EXPORT(testcase, "example_lptimer", utest_tc_init, utest_tc_cleanup, 30);
#endif

