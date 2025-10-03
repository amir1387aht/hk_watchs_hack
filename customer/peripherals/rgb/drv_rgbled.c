/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <board.h>
#include "bf0_hal_rcc.h"

/** @addtogroup bsp_driver Driver IO
  * @{
  */

/** @defgroup drv_rgbled RGBLED driver
  * @ingroup bsp_driver
  * @brief RGBLED BSP driver
  * @{
  */

#if defined(RGB_SK6812MINI_HS_ENABLE) || defined(_SIFLI_DOXYGEN_)

#include "drv_config.h"
#include "mem_section.h"

//#define DRV_DEBUG
#define LOG_TAG             "drv.rgb"
#include <drv_log.h>

struct rt_device_pwm *global_device = RT_NULL;

#define RGB_COLOR_LEN   24
#define RGB_REST_LEN   300
#define RGB_STOP_LEN   1

#define pwm_peroid  1600        // reg value =24
#define pulse_peroid  800

#define reg_high 18
#define reg_low  7
#define reg_end  50

struct rt_device_rgb;

struct rt_rgb_ops
{
    rt_err_t (*control)(struct rt_device_rgb *device, int cmd, void *arg);
};

struct rt_device_rgb
{
    struct rt_device parent;
    const struct rt_rgb_ops *ops;
};

#ifdef RGB_USING_SK6812MINI_HS_DEV_NAME
    #define RGBLED_NAME "rgbled"
#endif

L1_NON_RET_BSS_SECT_BEGIN(rgb_led_buf)
L1_NON_RET_BSS_SECT(rgb_led_buf, ALIGN(16) static uint16_t rgb_buff[RGB_REST_LEN + RGB_COLOR_LEN + RGB_STOP_LEN]);
L1_NON_RET_BSS_SECT_END

struct bf0_rgbled
{
    struct rt_device_rgb rgb_device;
    char *name;                         /*!<Device name*/
};

static struct bf0_rgbled bf0_rgbled_obj[] =
{
#ifdef RGB_USING_SK6812MINI_HS_DEV_NAME
    {.name = RGBLED_NAME}
#endif
};

static rt_err_t drv_rgbled_control(struct rt_device_rgb *device, int cmd, void *arg);
static struct rt_rgb_ops drv_ops =
{
    drv_rgbled_control
};

static void creater_color_array(uint32_t color)
{
    uint16_t i;
    rt_memset(rgb_buff, 0, sizeof(uint16_t) * RGB_REST_LEN + RGB_COLOR_LEN + 1);
    uint8_t red = (uint8_t)((color & 0x00ff0000) >> 16);
    uint8_t green = (uint8_t)((color & 0x0000ff00) >> 8);
    uint8_t blue = (uint8_t)((color & 0x000000ff) >> 0);

    for (i = 0; i < 8; i++)
    {
        if ((green << i) & 0x80)
        {
            rgb_buff[i + RGB_REST_LEN] = reg_high;
        }
        else
        {
            rgb_buff[i + RGB_REST_LEN] = reg_low;
        }
    }
    for (i = 0; i < 8; i++)
    {
        if ((red << i) & 0x80)
        {
            rgb_buff[i + 8 + RGB_REST_LEN] = reg_high;
        }
        else
        {
            rgb_buff[i + 8 + RGB_REST_LEN] = reg_low;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if ((blue << i) & 0x80)
        {
            rgb_buff[i + 16 + RGB_REST_LEN] = reg_high;
        }
        else
        {
            rgb_buff[i + 16 + RGB_REST_LEN] = reg_low;
        }
    }
    rgb_buff[RGB_REST_LEN + RGB_COLOR_LEN] = reg_end;

#ifdef DRV_DEBUG
    for (i = RGB_REST_LEN; i < RGB_REST_LEN + RGB_COLOR_LEN + RGB_STOP_LEN; i++)
    {
        rt_kprintf("%d,", rgb_buff[i]);
    }
#endif
    return;
}

/**
* @brief  rgbled update color.
* @param[in]  htim: GPT device object handle.
* @param[in]  configuration: GPT configuration, mainly GPT channel number, color.
* @retval RT_EOK if success, otherwise -RT_ERROR
*/
static rt_err_t drv_rgbled_update_color(struct bf0_rgbled *rgb_obj, struct rt_rgbled_configuration *configuration)
{

    creater_color_array(configuration->color_rgb);

    struct rt_pwm_configuration config;

    rt_memset((void *)&config, 0, sizeof(config));
#ifdef BSP_USING_RGBLED_CH
    config.channel = BSP_USING_RGBLED_CH;
#else
    LOG_I("NO CONFIG BSP_USING_RGBLED_CH");
#endif
    config.period = pwm_peroid;//1600
    config.pulse = pulse_peroid;//800
    //DMA is transferred to ccr
    config.dma_data = (rt_uint16_t *)rgb_buff;
    config.data_len = RGB_REST_LEN + RGB_REST_LEN + RGB_STOP_LEN;

    rt_device_control((struct rt_device *)global_device, PWM_CMD_SET, (void *)&config);
    rt_device_control((struct rt_device *)global_device, PWM_CMD_ENABLE, (void *)&config); //DMA -- rgb_buffer
    return RT_EOK;
}

/**
* @brief  rgbled controls.
* @param[in]  device: pwm device handle.
* @param[in]  cmd: control commands.
* @param[in]  arg: control command arguments.
* @retval RT_EOK if success, otherwise -RT_ERROR
*/
static rt_err_t drv_rgbled_control(struct rt_device_rgb *device, int cmd, void *arg)
{

    struct rt_rgbled_configuration *configuration = (struct rt_rgbled_configuration *)arg;

    struct bf0_rgbled *rgb_obj  = (struct bf0_rgbled *) device->parent.user_data;

    if ((RT_DEVICE_CTRL_RESUME != cmd) && (RT_DEVICE_CTRL_SUSPEND != cmd))
    {
        /* arg is not configuration for RESUME and SUSPEND command */
        // RT_ASSERT(configuration->color_rgb > 0); //Channel id must > 0
    }

    switch (cmd)
    {
    case PWM_CMD_SET_COLOR:
        drv_rgbled_update_color(rgb_obj, configuration);//通过传入的命令更新灯颜色
        break;
    default:
        break;
    }
    return RT_EOK;
}

/**
* @brief RGBLED device driver initialization.
* This is entry function of RGBLED device driver.
* @retval RT_EOK if success, otherwise -RT_ERROR
*/
static int bf0_rgbled_init(void)
{

    int i = 0;
    int result = RT_EOK;

#ifdef RGB_USING_SK6812MINI_HS_PWM_DEV_NAME
    //find pwm_device
    global_device = (struct rt_device_pwm *)rt_device_find(RGB_USING_SK6812MINI_HS_PWM_DEV_NAME);//find pwm
    if (!global_device)
    {
        LOG_I("find pwm err");
        return -RT_ERROR;
    }
#else
    LOG_D("NO CONFIG RGB_USING_SK6812MINI_HS_PWM_DEV_NAME");
#endif

    //register rgb_device
    bf0_rgbled_obj[0].rgb_device.parent.control = (rt_err_t (*)(rt_device_t, int, void *))drv_rgbled_control;
#if  1
    if (rt_device_register(&bf0_rgbled_obj[0].rgb_device.parent, bf0_rgbled_obj[0].name, RT_DEVICE_FLAG_RDWR))
    {
        LOG_E("%s register failed", RGBLED_NAME);
        result = -RT_ERROR;
    }
    else
    {
        LOG_D("%s register success", RGBLED_NAME);
    }
#endif

__exit:
    return result;
}
INIT_COMPONENT_EXPORT(bf0_rgbled_init);

// @} drv_pwm
// @} bsp_driver

#define DRV_TEST
#ifdef DRV_TEST

#include <finsh.h>

/** @addtogroup bsp_sample BSP driver sample commands.
  * @{
  */

/** @defgroup bsp_sample_pwm PWM sample commands
  * @ingroup bsp_sample
  * @brief PWM sample commands
  *
  * These sample commands demonstrate the usage of pwm driver.
  * User need to connect a buzzer to PWM output for testing.
  * @{
  */

/**
* @brief rgbled color display device.
* Usage: pwm_set [device] [channel] [period] [pulse]
*
* example: pwm_set pwm3 1 1000000 500
*/

static int rgb_color(int argc, char **argv)
{
    int result = 0;
    struct rt_device_pwm *rgbled_device;
    /*rgbled poweron*/
#ifdef SF32LB52X
    HAL_PMU_ConfigPeriLdo(PMU_PERI_LDO3_3V3, true, true);
#endif
    if (argc != 2)
    {
        LOG_I("Usage: rgb_color <rgb>");
        result = -RT_ERROR;
        goto _exit;
    }
    uint32_t color = atoh(argv[2]);

    rgbled_device = (struct rt_device_pwm *)rt_device_find("rgbled");
    if (!rgbled_device)
    {
        result = -RT_EIO;
        goto _exit;
    }
#ifdef SF32LB52X
    HAL_PIN_Set(PAD_PA32, GPTIM2_CH1, PIN_NOPULL, 1);   // RGB LED 52x
#elif SF32LB58X
    HAL_PIN_Set(PAD_PB39, GPTIM3_CH4, PIN_NOPULL, 1);   // RGB LED 58x
#elif  SF32LB56X
    HAL_PIN_Set(PAD_PB09, GPTIM3_CH4, PIN_NOPULL, 1);   //RGB LED 56x
#endif
    struct rt_rgbled_configuration configuration;
    configuration.color_rgb = color;
    rt_device_control(&rgbled_device->parent, PWM_CMD_SET_COLOR, &configuration);
_exit:
    return result;
}
MSH_CMD_EXPORT(rgb_color, rgb_color 0xffffff);

#endif /* DRV_TEST */

#endif /* RT_USING_PWM */

/// @} bsp_sample_pwm

/// @} bsp_sample

/// @} file

