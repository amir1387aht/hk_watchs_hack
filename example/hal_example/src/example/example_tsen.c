/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"
#include "tc_utils.h"

#ifdef HAL_TSEN_MODULE_ENABLED

/*
    This example demo:
        1. Read temperature in sync mode
        2. Read temperature in Async mode
*/

static TSEN_HandleTypeDef   TsenHandle;

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    HAL_TSEN_DeInit(&TsenHandle);
    LOG_I("Stop TSEN");
    return RT_EOK;
}

void TSEN_IRQHandler(void)
{
    ENTER_INTERRUPT();
    LOG_I("IRQ Fired");
    HAL_TSEN_IRQHandler(&TsenHandle);
    LEAVE_INTERRUPT();
}

static void testcase(int argc, char **argv)
{
    HAL_StatusTypeDef   status;
    int temperature;

    /*##-1- Initialize TSEN peripheral #######################################*/
    TsenHandle.Instance = hwp_tsen;
    if (HAL_TSEN_Init(&TsenHandle) == HAL_OK)
    {
        temperature = HAL_TSEN_Read(&TsenHandle);                                   /* Read synchronized*/
        LOG_I("Sync: Current temperature is %d degree\n", temperature);

        HAL_NVIC_SetPriority(TSEN_IRQn, 5, 0);                                      /* Set interrupt priority*/
        if (HAL_TSEN_Read_IT(&TsenHandle) == HAL_TSEN_STATE_BUSY)                   /* Read Async, interrupt will be enabled*/
            while (HAL_TSEN_GetState(&TsenHandle) != HAL_TSEN_STATE_READY);
        LOG_I("Async: Current temperature is %d degree\n", TsenHandle.temperature);
        uassert_true((TsenHandle.temperature > -40) && TsenHandle.temperature < 125);
    }
}

UTEST_TC_EXPORT(testcase, "example_tsen", utest_tc_init, utest_tc_cleanup, 10);

#endif /*HAL_CRC_MODULE_ENABLED*/

