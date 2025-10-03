/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <drv_common.h>

#define __GPIO_INSTANCE(GPIOx)  (GPIO##GPIOx##_BASE)

/** get driver pin id by instance id and its pin id
 *
 * e.g. GET_PIN(1, 0) for GPIO1 pin0 -->  driver pin id is 0
 *      GET_PIN(2, 0) for GPIO2 pin0 -->  driver pin id is 96
 *
 * @param  GPIOx GPIO instance id, starting from 0, 0 is for PBR, 1 is for GPIO1, 2 is for GPIO2, etc.
 * @param  PIN GPIO instance pin id, starting from 0
 * @return driver pin id, counting from 0
 */
#define GET_PIN(GPIOx,PIN)  ((GPIOx != 0) ? ((__GPIO_INSTANCE(GPIOx) == GPIO2_BASE) ? (GPIO1_PIN_NUM + (PIN)) : (PIN)) : (GPIO1_PIN_NUM + GPIO2_PIN_NUM + (PIN)))

#ifdef hwp_pbr
    /** get driver pin id by instance id and its pin id
    *
    * e.g. GET_PIN_2(hwp_gpio1, 0) for GPIO1 pin0 -->  driver pin id is 0
    *      GET_PIN_2(hwp_gpio2, 0) for GPIO2 pin0 -->  driver pin id is 96
    *
    * @param  GPIOx GPIO instance, hwp_pbr for PBR, hwp_gpio1 for GPIO1, hwp_gpio2 for GPIO2
    * @param  PIN GPIO instance pin id, starting from 0
    * @return driver pin id, counting from 0
    */
    #define GET_PIN_2(GPIOx,PIN)  ((GPIOx != hwp_pbr) ? (((GPIOx) == hwp_gpio2) ? (GPIO1_PIN_NUM + (PIN)) : (PIN)) : (GPIO1_PIN_NUM + GPIO2_PIN_NUM + (PIN)))

    /** get GPIO instance according to driver pin  */
    #define GET_GPIO_INSTANCE(PIN)  ((PIN) >= GPIO1_PIN_NUM ? (((PIN) < (GPIO1_PIN_NUM + GPIO2_PIN_NUM)) ? hwp_gpio2 : hwp_pbr) : hwp_gpio1)

    /** get GPIO instance pin id according to driver pin id */
    #define GET_GPIOx_PIN(PIN) ((PIN) >= GPIO1_PIN_NUM ? (((PIN) < (GPIO1_PIN_NUM + GPIO2_PIN_NUM)) ? ((PIN) - GPIO1_PIN_NUM) : ((PIN) - GPIO1_PIN_NUM - GPIO2_PIN_NUM)) : (PIN))

#else
    #define GET_PIN_2(GPIOx,PIN)  (((GPIOx) == hwp_gpio2) ? (GPIO1_PIN_NUM + (PIN)) : (PIN))

    #define GET_GPIO_INSTANCE(PIN)  ((PIN) >= GPIO1_PIN_NUM ? hwp_gpio2 : hwp_gpio1)

    #define GET_GPIOx_PIN(PIN) ((PIN) >= GPIO1_PIN_NUM ? (PIN) - GPIO1_PIN_NUM : (PIN))
#endif /* hwp_pbr */

int rt_hw_pin_init(void);

/**
 * @brief Trigger pin state change check
 *
 * @return void
 */
void drv_pin_check(void);

void drv_pin_irq_from_wsr(uint32_t wsr_pins);

#endif /* __DRV_GPIO_H__ */

