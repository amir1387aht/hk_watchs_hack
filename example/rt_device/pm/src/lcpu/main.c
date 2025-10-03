/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <stdlib.h>

#ifdef SF32LB55X
    #include "shell.h"
#endif /* SF32LB55X */

#define LOG_TAG "pm"
#include "log.h"

#ifndef LXT_LP_CYCLE
    #define LXT_LP_CYCLE 200
#endif
static rt_timer_t rc_10_time_handle;

static uint32_t g_rc_update_seconds = 15;

int cmd_lrc(int argc, char **argv)
{
    if (argc > 1)
    {
        g_rc_update_seconds = atoi(argv[1]);
    }
    if (rc_10_time_handle)
    {
        rt_kprintf("rc10 update every %ds\n", g_rc_update_seconds);
        if (argc > 1)
        {
            rt_tick_t ticks = g_rc_update_seconds * 1000;
            rt_timer_control(rc_10_time_handle, RT_TIMER_CTRL_SET_TIME, &ticks);
        }
    }
    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_lrc, __cmd_lrc, set RC update internal seconds);

void rc10k_timeout_handler(void *parameter)
{
    HAL_RC_CAL_update_reference_cycle_on_48M(LXT_LP_CYCLE);
}

static void wake_key_handle(void)
{
    rt_kprintf("Lcpu wake_key_handle!!!\n");
}

static void lcpu_wakeup(void)
{
    uint8_t pin;
#if defined(SF32LB55X)
#define WAKE_KEY (96+48) //PB48 #WKUP_PIN5

    HAL_LPAON_EnableWakeupSrc(LPAON_WAKEUP_SRC_PIN5, AON_PIN_MODE_NEG_EDGE);

    rt_pin_mode(WAKE_KEY, PIN_MODE_INPUT);

    rt_pin_attach_irq(WAKE_KEY, PIN_IRQ_MODE_FALLING, (void *) wake_key_handle,
                      (void *)(rt_uint32_t) WAKE_KEY);
    rt_pin_irq_enable(WAKE_KEY, 1);

#elif defined(SF32LB56X)

#else

#endif

}

int main(void)
{
    rt_pm_request(PM_SLEEP_MODE_IDLE);
    rt_kprintf("lcpu main!!!\n");
    rt_thread_delay(3000);
    lcpu_wakeup();
    rt_pm_release(PM_SLEEP_MODE_IDLE);

    if (HAL_PMU_LXT_DISABLED())
    {
        lpsys_clk_setting_t clk_setting;

        drv_get_lpsys_clk(&clk_setting);
        /* hclk must be 48MHz if RC10k is used */
        RT_ASSERT(48000000 == clk_setting.hclk);
        HAL_RC_CAL_update_reference_cycle_on_48M(LXT_LP_CYCLE);

        rc_10_time_handle  = rt_timer_create("rc10k", rc10k_timeout_handler,  NULL,
                                             rt_tick_from_millisecond(g_rc_update_seconds * 1000), RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
        RT_ASSERT(rc_10_time_handle);
        rt_timer_start(rc_10_time_handle);
    }

    //HAL_RCC_LCPU_SetDiv(2, );
    while (1)
    {
        rt_thread_mdelay(20000);
        rt_kprintf("lcpu timer wakeup!!!\n");
    }
    return RT_EOK;
}

