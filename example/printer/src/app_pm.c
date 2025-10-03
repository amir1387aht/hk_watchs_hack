/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_common.h"
#include "mem_section.h"
#define LOG_TAG "app_pm"
#include "log.h"

static void pm_shutdown_config(void)
{
#ifdef SOC_BF0_HCPU
#if defined(SF32LB52X)

    //rtc:  0:PE disable; 1: PE enable; PA35~PA44 Enable PE, pull down; PA34 disable pull down;
    HAL_PIN_Set(PAD_PA34, GPIO_A34, PIN_NOPULL, 1);
    for (int i = PAD_PA35; i <= PAD_PA44; i++)
    {
        HAL_PIN_Set(PAD_PA35 + i, GPIO_A35 + i, PIN_PULLDOWN, 1);
    }
    //Hibernate Wakeup support PIN0 and PIN1;
#define PIN0    0
#define PIN1    1
#if defined(BSP_KEY1_PIN) &&  (BSP_KEY1_PIN != -1)
    HAL_PMU_SelectWakeupPin(PIN0, HAL_HPAON_QueryWakeupPin(hwp_gpio1, BSP_KEY1_PIN)); //PIN0 select:PA34 as pin0  PA24=>0 PA30=>6  PA34=>10  PA35=>11
    HAL_PMU_EnablePinWakeup(PIN0, AON_PIN_MODE_HIGH);  //PIN0 wake mode: AS 0: high level
#endif
    HAL_PMU_EnableRtcWakeup();
#endif
#endif
}

L1_RET_CODE_SECT(pm_shutdown, void pm_shutdown(void))
{
    rt_kprintf("%s, hibernate", __func__);
    pm_shutdown_config();
    HAL_Delay_us(3000000);  /**/
    rt_hw_interrupt_disable();
#if defined(SF32LB52X)
    HAL_PMU_ConfigPeriLdo(PMUC_PERI_LDO_EN_VDD33_LDO2_Pos, false, false);
    HAL_PMU_ConfigPeriLdo(PMU_PERI_LDO_1V8, false, false);
#endif
    HAL_PMU_EnterHibernate();
    while (1);
}
#ifdef RT_USING_FINSH
static int cmd_pwr_off(int argc, char **argv)
{
    pm_shutdown();
    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_pwr_off, __cmd_pwr_off, power off);
#endif

