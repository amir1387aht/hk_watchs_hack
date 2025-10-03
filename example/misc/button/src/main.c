/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "button.h"
#include <board.h>

/*
    action:
    BUTTON_PRESSED  = 0,    Indicates that a button is pressed
    BUTTON_RELEASED = 1,    Indicates that a button is released
    BUTTON_LONG_PRESSED = 2, Indicates that a button is long released
    BUTTON_CLICKED  = 3,     Indicates that a button is clicked
*/
static void button_event_handler(int32_t pin, button_action_t action)
{
    rt_kprintf("button:%d,%d\n", pin, action);
}

int main(void)
{
#if defined(BSP_USING_BOARD_SF32LB52_LCD_N16R8)
    HAL_PIN_Set(PAD_PA34, GPIO_A34, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_PA11, GPIO_A11, PIN_PULLDOWN, 1);
#elif defined (BSP_USING_BOARD_SF32LB58_LCD_N16R64N4)
    HAL_PIN_Set(PAD_PB56, GPIO_B56, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_PB54, GPIO_B54, PIN_PULLDOWN, 1);
#endif
    /*
    587 board PB pin needs to add 96
    54+96->150
    56+96->152
    */

    button_cfg_t cfg;

#if defined(BSP_USING_BOARD_SF32LB52_LCD_N16R8)
    cfg.pin = 34;
#elif defined (BSP_USING_BOARD_SF32LB58_LCD_N16R64N4)
    cfg.pin = 152;
#endif
    cfg.active_state = BUTTON_ACTIVE_HIGH;
    cfg.mode = PIN_MODE_INPUT;
    cfg.button_handler = button_event_handler;
    int32_t id = button_init(&cfg);
    RT_ASSERT(id >= 0);
    RT_ASSERT(SF_EOK == button_enable(id));

#if defined(BSP_USING_BOARD_SF32LB52_LCD_N16R8)
    cfg.pin = 11;
#elif defined (BSP_USING_BOARD_SF32LB58_LCD_N16R64N4)
    cfg.pin = 150;
#endif
    cfg.active_state = BUTTON_ACTIVE_HIGH;
    cfg.mode = PIN_MODE_INPUT;
    cfg.button_handler = button_event_handler;
    id = button_init(&cfg);
    RT_ASSERT(id >= 0);
    RT_ASSERT(SF_EOK == button_enable(id));

    while (1)
    {
        rt_thread_mdelay(1000000);
    }

    return RT_EOK;
}

