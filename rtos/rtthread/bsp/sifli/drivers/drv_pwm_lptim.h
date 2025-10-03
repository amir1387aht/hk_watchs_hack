/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_PWM_LPTIM_H__
#define __DRV_PWM_LPTIM_H__
#include "drv_config.h"
#include "bf0_hal_lptim.h"
#if defined(BSP_USING_PWM_LPTIM1) ||defined(BSP_USING_PWM_LPTIM2)||defined(BSP_USING_PWM_LPTIM3)|| defined(_SIFLI_DOXYGEN_)

#define MAX_PERIOD 65535
#define MIN_PERIOD 3
#define MIN_PULSE 2

struct bf0_pwm_lp
{
    struct rt_device_pwm    pwm_device;    /*!<PWM device object handle*/
    LPTIM_HandleTypeDef     tim_handle;    /*!<LPTimer device object handle used in PWM*/
    struct rt_pwm_configuration config;     /*!<PWM configuration*/
    char *name;                         /*!<Device name*/
};

typedef enum
{
    LPTIME_PWM_CLK_SOURCE_USING_LPCLK       = 0,
    LPTIME_PWM_CLK_SOURCE_USING_APBCLK      = 1,
    LPTIME_PWM_CLK_SOURCE_USING_EXTER_CLK   = 2,
    LPTIME_PWM_CLK_SOURCE_CNT
} Lptime_pwm_ClockSource_Enum_T;

#endif


#endif
