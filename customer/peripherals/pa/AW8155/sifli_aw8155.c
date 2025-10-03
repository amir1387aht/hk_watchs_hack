/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "drv_gpio.h"
#if defined(PMIC_CTRL_ENABLE)
    #include "pmic_controller.h"
#endif
#define DBG_TAG           "aw8155"
#include "rtdbg.h"


#define  AW8155_WORK_MODE   1   // only 1 2 3 4

static GPIO_TypeDef *gpio_inst = GET_GPIO_INSTANCE(AW8155_GPIO_PIN);
static uint16_t gpio_pin = GET_GPIOx_PIN(AW8155_GPIO_PIN);

void aw8155_gpio_write(bool State)
{
    GPIO_PinState PinState = (GPIO_PinState)State;
    HAL_GPIO_WritePin(gpio_inst, gpio_pin, PinState);
    HAL_Delay_us(5);
}

static int aw8155_mode = AW8155_WORK_MODE;
void sifli_aw8155_start()
{
    int i = aw8155_mode;
    GPIO_InitTypeDef GPIO_InitStruct;
#if defined(PMIC_CTRL_ENABLE)
    pmic_device_control(PMIC_OUT_VBAT_HVSW150_1, 1, 1);//wait ???
#else
    /* @todo power handle */
    rt_kprintf("sifli_aw8155 to do power handle \n");
#endif
    // set sensor pin to output mode
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pin  = gpio_pin;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(gpio_inst, &GPIO_InitStruct);
    HAL_Delay_us(550);

    while (1)
    {
        aw8155_gpio_write(1);
        i--;
        if (i > 0)
        {
            aw8155_gpio_write(0);
        }
        else
        {
            break;
        }
    }
    rt_kprintf("sifli_aw8155_start,mode:%d %d\n", aw8155_mode, AW8155_GPIO_PIN);

}

void sifli_aw8155_stop()
{
    aw8155_gpio_write(0);
    HAL_Delay_us(550);
#if defined(PMIC_CTRL_ENABLE)
    pmic_device_control(PMIC_OUT_VBAT_HVSW150_1, 0, 1);
#else
    /* @todo power handle */
#endif
    rt_kprintf("sifli_aw8155_stop \n");
}

int apa_set_mode(int argc, char *argv[])
{
    rt_thread_t thread;
    int mode;

    if (argc != 2)
    {
        rt_kprintf("arg para num error\n");
        return -1;
    }
    mode = strtol(argv[1], NULL, 10);
    rt_kprintf("analog PA 8155 work mode=%d\n", mode);

    if ((mode >= 1) && (mode <= 4))
    {
        aw8155_mode = mode;
    }
    else
    {
        rt_kprintf("analog PA mode error \n");
    }


    return 0;
}

MSH_CMD_EXPORT(apa_set_mode, set analog pa mode);



