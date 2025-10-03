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
    #include "ble.h"
    #include "shell.h"
#endif /* SF32LB55X */

#define LOG_TAG "ble_app"
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

int main(void)
{
    int count = 0;
#ifdef SF32LB55X
    app_env_t *env = ble_app_get_env();
#endif /* SF32LB55X */

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

#ifdef SF32LB55X
    env->mb_handle = rt_mb_create("app", 8, RT_IPC_FLAG_FIFO);
    sifli_ble_enable();
    env->time_handle  = rt_timer_create("app", app_timeout_handler,  NULL,
                                        rt_tick_from_millisecond(BLE_APP_TIMEOUT_INTERVAL), RT_TIMER_FLAG_SOFT_TIMER);
#endif /* SF32LB55X */

    //HAL_RCC_LCPU_SetDiv(2, );
    while (1)
    {
#ifdef SF32LB55X
        uint32_t value;
        int ret;
        rt_mb_recv(env->mb_handle, (rt_uint32_t *)&value, RT_WAITING_FOREVER);
        if (value == BLE_POWER_ON_IND)
        {
            env->is_power_on = 1;
            env->conn_para.mtu = 23; /* Default value. */
            ble_app_service_init();
            ble_app_advertising_start();
            LOG_I("receive BLE power on!\r\n");

            do
            {
                rt_device_t console = rt_console_get_device();
                if (console)
                {
                    //   finsh_set_device(console->parent.name);
                }
            }
            while (0);
        }
#else
        rt_thread_mdelay(1000000);
#endif /* SF32LB55X */
    }
    return RT_EOK;
}

