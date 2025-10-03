/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"

#ifdef HAL_RNG_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define BUFFER_SIZE    114

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void test_rng(void)
{
    /* RNG handler declaration */
    RNG_HandleTypeDef   RngHandle;
    HAL_StatusTypeDef   status;
    uint32_t            value;

    /*##-1- Initialize RNG peripheral #######################################*/
    RngHandle.Instance = hwp_trng;

    if (HAL_RNG_Init(&RngHandle) != HAL_OK)
    {
        /* Initialization Error */
        uassert_true(RT_FALSE);
    }

    /*##-2- Generate seed ###################################*/
    value = 0;
    status = HAL_RNG_Generate(&RngHandle, &value,  1);
    LOG_I("Generate seed value=%x, status=%d", value, status);
    uassert_true(value != 0);

    /*##-3- Generate random ###################################*/
    value = 0;
    status = HAL_RNG_Generate(&RngHandle, &value, 0);
    LOG_I("Generate Random value=%x, status=%d", value, status);
    uassert_true(value != 0);
}

static void testcase(int argc, char **argv)
{
    UTEST_UNIT_RUN(test_rng);
}

UTEST_TC_EXPORT(testcase, "example_rng", utest_tc_init, utest_tc_cleanup, 10);

#endif /*HAL_CRC_MODULE_ENABLED*/

