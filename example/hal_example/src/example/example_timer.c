/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdlib.h>
#include "utest.h"
#include "board.h"
#include "tc_utils.h"

#ifdef HAL_GPT_MODULE_ENABLED

/*
    This example demo:
        1. Use GPTIM4 to generate interrupt in 3.5 seconds
        2. Use BTIM3 to generate interrupt in 3.5 seconds
*/

typedef struct hal_hwtimerval
{
    uint32_t sec;      /* second */
    uint32_t usec;     /* microsecond */
} hal_hwtimerval_t;

static GPT_HandleTypeDef TIM_Handle = {0};
rt_uint32_t ticks_start = 0;

#define FREQENCY 10000
static HAL_StatusTypeDef timer_init(GPT_HandleTypeDef *tim, GPT_TypeDef *instance, IRQn_Type irq, uint32_t period, int core_id)
{
    HAL_StatusTypeDef r = HAL_OK;

    tim->Instance = instance;
    tim->Init.Prescaler = HAL_RCC_GetPCLKFreq(core_id, 1) / FREQENCY; /*Prescaler is 16 bits, please select correct frequency*/
    tim->core = core_id;
    tim->Init.CounterMode = GPT_COUNTERMODE_UP;         /*GPTIM could support counter up/down, BTIM only support count up*/
    tim->Init.RepetitionCounter = 0;
    tim->Init.Period = period;
    LOG_I("Prescaler:%d PCLK:%d period:%d\n", tim->Init.Prescaler, HAL_RCC_GetPCLKFreq(core_id, 1), period);

    //if (HAL_GPT_OnePulse_Init(tim, GPT_OPMODE_SINGLE) == HAL_OK)
    if (HAL_GPT_Base_Init(tim) == HAL_OK)
    {
        HAL_NVIC_SetPriority(irq, 3, 0);                /* set the TIMx priority */
        HAL_NVIC_EnableIRQ(irq);                        /* enable the TIMx global Interrupt */
        __HAL_GPT_CLEAR_FLAG(tim, GPT_FLAG_UPDATE);     /* clear update flag */
        __HAL_GPT_URS_ENABLE(tim);                      /* enable update request source */
    }
    else
    {
        LOG_E("Timer init error");
        r = HAL_ERROR;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef timer_start(GPT_HandleTypeDef *tim)
{
    HAL_StatusTypeDef result = HAL_OK;

    if (HAL_GPT_Base_Start_IT(tim) != HAL_OK)           /* start timer */
    {
        LOG_E("Timer start error");
        result = HAL_ERROR;
    }
    return result;
}

void GPTIM4_IRQHandler(void)
{
    ENTER_INTERRUPT();
    LOG_I("GPTIM4 timeout");
    HAL_GPT_IRQHandler(&TIM_Handle);
    LEAVE_INTERRUPT();
}

void BTIM3_IRQHandler(void)
{
    ENTER_INTERRUPT();
    LOG_I("BTIM3 timeout %d\n", rt_tick_get() - ticks_start);
    HAL_GPT_IRQHandler(&TIM_Handle);
    LEAVE_INTERRUPT();
}

void ATIM1_IRQHandler(void)
{
    ENTER_INTERRUPT();
    LOG_I("ATIM1 timeout %d\n", rt_tick_get() - ticks_start);
    HAL_GPT_IRQHandler(&TIM_Handle);
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
    HAL_StatusTypeDef ret;

#ifdef hwp_gptim4
    LOG_I("Test GPTIM4: Timeout in 3.5 seconds");
    timer_init(&TIM_Handle, hwp_gptim4, GPTIM4_IRQn, 3500 * FREQENCY / 1000, CORE_ID_LCPU);
    timer_start(&TIM_Handle);
    rt_thread_mdelay(4000);
    HAL_GPT_Base_DeInit(&TIM_Handle);
#endif
#ifdef hwp_btim3
    LOG_I("Test BTIM3: Timeout in 3.5 seconds");
    timer_init(&TIM_Handle, (GPT_TypeDef *)hwp_btim3, BTIM3_IRQn, 3500 * FREQENCY / 1000, CORE_ID_LCPU);
    timer_start(&TIM_Handle);
    ticks_start = rt_tick_get();
    rt_thread_mdelay(4000);
    HAL_GPT_Base_DeInit(&TIM_Handle);
#endif

#ifdef hwp_atim1
    LOG_I("Test atimer1: Timeout in 3.5 seconds");
    memset(&TIM_Handle, 0, sizeof(TIM_Handle));
    timer_init(&TIM_Handle, (GPT_TypeDef *)hwp_atim1, ATIM1_IRQn, 3500 * FREQENCY / 1000, CORE_ID_HCPU);
    timer_start(&TIM_Handle);
    ticks_start = rt_tick_get();
    rt_thread_mdelay(4000);
    HAL_GPT_Base_DeInit(&TIM_Handle);
#endif
}

UTEST_TC_EXPORT(testcase, "example_timer", utest_tc_init, utest_tc_cleanup, 30);

#endif

