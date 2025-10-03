/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <board.h>

/** @addtogroup bsp_driver Driver IO
  * @{
  */

/** @defgroup drv_nnacc NN Accelerator
  * @brief Neural Network Accelerator BSP driver
  * @{
  */

#ifdef BSP_USING_NN_ACC
#include <drv_nnacc.h>
#include <drv_common.h>
#include <string.h>

#define  DBG_LEVEL            DBG_ERROR  //DBG_LOG //
#define LOG_TAG              "drv.nnacc"
#include <drv_log.h>

static NNACC_HandleTypeDef nn_acc = {.instance = hwp_nnacc};

void NNACC_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_NNACC_IRQHandler(&nn_acc);
    rt_interrupt_leave();
}

rt_err_t nn_acc_start(NNACC_ConfigTypeDef *config)
{
    HAL_StatusTypeDef ret;

    ret = HAL_NNACC_Start(&nn_acc, config);

    if (HAL_OK == ret)
    {
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}

rt_err_t nn_acc_start_IT(NNACC_ConfigTypeDef *config, nn_acc_cbk cbk)
{
    HAL_StatusTypeDef ret;

#ifdef SOC_BF0_HCPU
    NVIC_EnableIRQ(NNACC_IRQn);
#else
    NVIC_EnableIRQ(NNACC2_IRQn);
#endif

    nn_acc.CpltCallback = (void (*)(NNACC_HandleTypeDef *))cbk;
    ret = HAL_NNACC_Start_IT(&nn_acc, config);
    RT_ASSERT(HAL_OK == ret);

    if (HAL_OK == ret)
    {
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}

NNACC_HandleTypeDef *drv_get_nnacc_handle()
{
    return &nn_acc;
}

int nn_acc_init(void)
{
    HAL_StatusTypeDef ret = RT_ERROR;

    ret = HAL_NNACC_Init(&nn_acc);

    if (HAL_OK == ret)
    {
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}
INIT_BOARD_EXPORT(nn_acc_init);

#endif /* BSP_USING_NN_ACC */

/// @} drv_nnacc
/// @} bsp_driver
/// @} file

