/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "board.h"
#ifdef BSP_USING_PM
    #include "bf0_pm.h"
    #include "drv_gpio.h"
#endif /* BSP_USING_PM */
#include "button.h"

#ifdef BSP_KEY1_ACTIVE_HIGH
    #define BUTTON_ACTIVE_POL BUTTON_ACTIVE_HIGH
#else
    #define BUTTON_ACTIVE_POL BUTTON_ACTIVE_LOW
#endif

static rt_timer_t rc10k_time_handle;
static int32_t key1_button_handle;

void button_event_handler(int32_t pin, button_action_t button_action)
{
    rt_kprintf("pin: %d, action: %d\n", pin, button_action);
}

static void init_pin(void)
{
#if (BSP_KEY1_PIN >= GPIO1_PIN_NUM)
    button_cfg_t cfg;
#if defined(BSP_USING_PM)
    int8_t wakeup_pin;
    uint16_t gpio_pin;
    GPIO_TypeDef *gpio;
#endif /* BSP_USING_PM */

    cfg.pin = BSP_KEY1_PIN;
    cfg.active_state = BUTTON_ACTIVE_POL;
    cfg.mode = PIN_MODE_INPUT;
    cfg.button_handler = button_event_handler;
    int32_t id = button_init(&cfg);
    RT_ASSERT(id >= 0);
    RT_ASSERT(SF_EOK == button_enable(id));
    key1_button_handle = id;

#if defined(BSP_USING_PM)
    gpio = GET_GPIO_INSTANCE(BSP_KEY1_PIN);
    gpio_pin = GET_GPIOx_PIN(BSP_KEY1_PIN);

    wakeup_pin = HAL_LPAON_QueryWakeupPin(gpio, gpio_pin);
    RT_ASSERT(wakeup_pin >= 0);

    pm_enable_pin_wakeup(wakeup_pin, AON_PIN_MODE_DOUBLE_EDGE);
#endif /* BSP_USING_PM */

#endif /* BSP_KEY1_PIN < GPIO1_PIN_NUM */
}

void rc10k_timeout_handler(void *parameter)
{
    if (HAL_LXT_DISABLED())
    {
        HAL_RC_CAL_update_reference_cycle_on_48M(LXT_LP_CYCLE);
    }
    else
    {
        rt_timer_stop(rc10k_time_handle);
    }
}

int main(void)
{
    init_pin();
    if (HAL_LXT_DISABLED())
    {
        rc10k_time_handle  = rt_timer_create("rc10", rc10k_timeout_handler,  NULL,
                                             rt_tick_from_millisecond(15 * 1000), RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER); // 15s
        RT_ASSERT(rc10k_time_handle);
        rt_timer_start(rc10k_time_handle);
    }

    while (1)
    {
        rt_thread_mdelay(3000);
    }
    return RT_EOK;
}

