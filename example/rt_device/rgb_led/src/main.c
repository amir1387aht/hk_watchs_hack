#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "drivers/rt_drv_pwm.h"

#define RGB_COLOR   (0x00ff00)

#define RGBLED_NAME    "rgbled"


struct rt_device *rgbled_device;
struct rt_color
{
    char *color_name;
    uint32_t color;
};

struct rt_color rgb_color_arry[] =
{
    {"black", 0x000000},
    {"blue", 0x0000ff},
    {"green", 0x00ff00},
    {"cyan",  0x00ffff},
    {"red", 0xff0000},
    {"purple", 0xff00ff},
    {"yellow", 0xffff00},
    {"white", 0xffffff}
};


void rgb_led_init()
{
    /*rgbled poweron*/
#ifdef SF32LB52X
    HAL_PMU_ConfigPeriLdo(PMU_PERI_LDO3_3V3, true, true);
#endif
    rgbled_device = rt_device_find(RGBLED_NAME);//find rgb
    if (!rgbled_device)
    {
        RT_ASSERT(0);
    }
}

void rgb_led_set_color(uint32_t color)
{
#ifdef SF32LB52X
    HAL_PIN_Set(PAD_PA32, GPTIM2_CH1, PIN_NOPULL, 1);   // RGB LED 52x  pwm3_cc1
#elif defined SF32LB58X
    HAL_PIN_Set(PAD_PB39, GPTIM3_CH4, PIN_NOPULL, 0);//58x          pwm4_cc4
#elif defined SF32LB56X
    HAL_PIN_Set(PAD_PB09, GPTIM3_CH4, PIN_NOPULL, 0);//566
#endif
    struct rt_rgbled_configuration configuration;
    configuration.color_rgb = color;
    rt_device_control(rgbled_device, PWM_CMD_SET_COLOR, &configuration);
}


void rgb_color_array_display()
{
    uint16_t i = 0;
    rt_kprintf("start display color!\n");
    while (1)
    {
        if (i < sizeof(rgb_color_arry) / sizeof(struct rt_color))
        {
            rt_kprintf("-> %s\n", rgb_color_arry[i].color_name);
            rgb_led_set_color(rgb_color_arry[i].color);
            rt_thread_mdelay(1000);

        }
        i++;
        if (i >= 8)
            i = 0;

    }
}
/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
int main(void)
{
    /* Output a message on console using printf function */
    rt_kprintf("Hello world!\n");
    rgb_led_init();
    //rgb_color_auto();
    rgb_color_array_display();
    /* Infinite loop */
    while (1)
    {
        rt_thread_mdelay(1);
    }
    return 0;
}

