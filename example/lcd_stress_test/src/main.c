/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include "drv_lcd_private.h"
#include "mem_section.h"

#define TEST_FRAMEBUFFER

static struct rt_device_graphic_info lcd_info;
static rt_sem_t p_async_draw_sema = NULL;
static rt_err_t (*ancestor_tx_complete)(rt_device_t dev, void *buffer);
static uint32_t loop_cnt;
static uint32_t running_task;

#ifdef TEST_FRAMEBUFFER
    L2_NON_RET_BSS_SECT_BEGIN(frambuf)
    L2_NON_RET_BSS_SECT(frambuf, ALIGN(64) static char frambuffer[LCD_HOR_RES_MAX * LCD_VER_RES_MAX * 3]);
    L2_NON_RET_BSS_SECT_END

#else
    #ifdef PSRAM_BASE
        const char *frambuffer = (const char *)PSRAM_BASE;
    #else
        const char *frambuffer = (const char *)HPSYS_RAM0_BASE;
    #endif /* PSRAM_BASE */
#endif

static void dummy(void) {}

static uint32_t loop_end(void)
{
    uint32_t retv;
    rt_enter_critical();

    if (loop_cnt > 0)
    {
        retv = loop_cnt;

        loop_cnt--;
    }
    else
    {
        retv = 0;
    }
    rt_kprintf("****loop %d ****\n", retv);

    rt_exit_critical();
    return retv;
}

static void task_cleanup(struct rt_thread *tid)
{
    rt_base_t mask = rt_hw_interrupt_disable();
    if (running_task > 0) running_task--;
    rt_hw_interrupt_enable(mask);

}

static uint32_t rand_in_range(uint32_t min, uint32_t max)
{
    return  min + (rand() % (max - min + 1));
}

static uint32_t make_color(uint16_t cf, uint32_t rgb888)
{
    uint8_t r, g, b;

    r = (rgb888 >> 16) & 0xFF;
    g = (rgb888 >> 8) & 0xFF;
    b = (rgb888 >> 0) & 0xFF;

    switch (cf)
    {
    case RTGRAPHIC_PIXEL_FORMAT_RGB565:
        return ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3));
    case RTGRAPHIC_PIXEL_FORMAT_RGB666:
        return ((((r) & 0xFC) << 10) | (((g) & 0xFC) << 4) | (((b) & 0xFC) >> 2));
    case RTGRAPHIC_PIXEL_FORMAT_RGB888:
        return ((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF));

    default:
        return 0;
    }
}

static void fill_color(uint8_t *buf, uint32_t width, uint32_t height,
                       uint16_t cf, uint32_t ARGB8888)
{
    uint8_t pixel_size;

    if (RTGRAPHIC_PIXEL_FORMAT_RGB565 == cf)
    {
        pixel_size = 2;
    }
    else if (RTGRAPHIC_PIXEL_FORMAT_RGB888 == cf)
    {
        pixel_size = 3;
    }
    else
    {
        RT_ASSERT(0);
    }

    uint32_t i, j, k, c;
    c = make_color(cf, ARGB8888);
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            for (k = 0; k < pixel_size; k++)
            {
                *buf++ = (c >> (k << 3)) & 0xFF;
            }
        }
    }
}

static rt_err_t lcd_flush_done(rt_device_t dev, void *buffer)
{

    if (ancestor_tx_complete) ancestor_tx_complete(dev, buffer);

    if (p_async_draw_sema) rt_sem_release(p_async_draw_sema);
    return RT_EOK;
}

static void set_brightness(rt_device_t lcd_device)
{
    rt_err_t err = rt_device_open(lcd_device, RT_DEVICE_OFLAG_RDWR);
    if ((RT_EOK == err) || (-RT_EBUSY == err))
    {
        uint8_t brightness = (uint8_t) rand_in_range(50, 100);
        rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &brightness);

        if (RT_EOK == err) rt_device_close(lcd_device);
    }
}

static void draw(rt_device_t lcd_device, uint8_t async_draw, uint8_t random_area)
{
    int32_t dx, dy, w, h;

#ifdef BSP_USING_RAMLESS_LCD
    random_area = 0; //Only full screen FB supported on RAMLESS screen
#endif /* BSP_USING_RAMLESS_LCD */
    if (random_area)
    {
        dx = (int32_t) rand_in_range(0, lcd_info.width - lcd_info.draw_align);
        dy = (int32_t) rand_in_range(0, lcd_info.height - lcd_info.draw_align);

        w  = (int32_t) rand_in_range(lcd_info.draw_align, lcd_info.width - dx);
        h  = (int32_t) rand_in_range(lcd_info.draw_align, lcd_info.height - dy);

        /*In case of some LCD need align to 2 or 4 pixel*/
        dx = RT_ALIGN_DOWN(dx, lcd_info.draw_align);
        dy = RT_ALIGN_DOWN(dy, lcd_info.draw_align);

        w  = RT_ALIGN_DOWN(w, lcd_info.draw_align);
        h  = RT_ALIGN_DOWN(h, lcd_info.draw_align);
    }
    else
    {
        //Draw full screen
        dx = 0;
        dy = 0;
        w = lcd_info.width;
        h = lcd_info.height;
    }

#ifdef TEST_FRAMEBUFFER
    fill_color((uint8_t *)frambuffer, lcd_info.width, lcd_info.height,
               (16 == lcd_info.bits_per_pixel) ? RTGRAPHIC_PIXEL_FORMAT_RGB565 : RTGRAPHIC_PIXEL_FORMAT_RGB888,
               rand_in_range(0x000000, 0xFFFFFF));
#endif /* TEST_FRAMEBUFFER */

    if (w && h)
    {
        rt_graphix_ops(lcd_device)->set_window(dx, dy, dx + w - 1, dy + h - 1);

        set_brightness(lcd_device);

        if (async_draw)
        {
            if (p_async_draw_sema)
            {
                rt_sem_control(p_async_draw_sema, RT_IPC_CMD_RESET, (void *)1);
                rt_sem_take(p_async_draw_sema, RT_WAITING_FOREVER);
            }
            rt_graphix_ops(lcd_device)->draw_rect_async((const char *)frambuffer, dx, dy, dx + w - 1, dy + h - 1);
        }
        else
            rt_graphix_ops(lcd_device)->draw_rect((const char *)frambuffer, dx, dy, dx + w - 1, dy + h - 1);
    }
}

static void standby_sleep_wakeup(rt_device_t lcd_device)
{
#ifdef RT_USING_PM
    rt_pm_release(PM_SLEEP_MODE_IDLE);
    rt_pm_hw_device_stop();

    rt_thread_mdelay((rt_int32_t)rand_in_range(500, 1000));

    rt_pm_request(PM_SLEEP_MODE_IDLE);
    rt_pm_hw_device_start();
#else
    rt_device_control(lcd_device, RT_DEVICE_CTRL_SUSPEND, NULL);
    HAL_RCC_ResetModule(RCC_MOD_LCDC1);  //Reset LCDC DSI DSI-PHY etc. to act like entry real standby
#ifdef HAL_DSI_MODULE_ENABLED
    HAL_RCC_ResetModule(RCC_MOD_DSIHOST);
    HAL_RCC_ResetModule(RCC_MOD_DSIPHY);
#endif /* HAL_DSI_MODULE_ENABLED */
    rt_device_control(lcd_device, RT_DEVICE_CTRL_RESUME, NULL);
#endif  /* RT_USING_PM */
}

static void brightness_set_task(void *parameter)
{
    rt_device_t device = (rt_device_t) parameter;

    while (loop_end())
    {
        set_brightness(device);

        rt_thread_mdelay((rt_int32_t)rand_in_range(16, 100));
    }
}

static void async_draw_task(void *parameter)
{
    rt_device_t lcd_device = (rt_device_t) parameter;

    while (loop_end())
    {
        draw(lcd_device, 1, 1);
        rt_thread_mdelay((rt_int32_t)rand_in_range(16, 100));

    }
}

static void sync_draw_task(void *parameter)
{
    rt_device_t lcd_device = (rt_device_t) parameter;

    while (loop_end())
    {
        draw(lcd_device, 0, 1);
        rt_thread_mdelay((rt_int32_t)rand_in_range(16, 100));

    }
}

static void draw_close_open_task(void *parameter)
{
    rt_device_t lcd_device = (rt_device_t) parameter;
    bool lcd_drawing;
    uint8_t idle_mode_on;

    while (loop_end())
    {
        rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_BUSY, &lcd_drawing);
        if (!lcd_drawing)
        {

            //rt_thread_mdelay((rt_int32_t)rand_in_range(1, 100));
            rt_device_close(lcd_device);
            //rt_thread_mdelay((rt_int32_t)rand_in_range(1, 100));
            RT_ASSERT(RT_EOK == rt_device_open(lcd_device, RT_DEVICE_OFLAG_RDWR));

            //rt_thread_mdelay((rt_int32_t)rand_in_range(1, 100));

            uint16_t framebuffer_color_format = RTGRAPHIC_PIXEL_FORMAT_RGB565;
            rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BUF_FORMAT, &framebuffer_color_format);
        }
        draw(lcd_device, 0, 1);
        rt_thread_mdelay(1000);//Make sure to see what we drawn
    }
}

static void draw_onoff_sleep_task(void *parameter)
{
    rt_device_t lcd_device = (rt_device_t) parameter;
    bool lcd_drawing;
    uint8_t idle_mode_on;

    while (loop_end())
    {
        rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_BUSY, &lcd_drawing);
        if (!lcd_drawing)
        {

            rt_device_control(lcd_device, RTGRAPHIC_CTRL_POWEROFF, NULL);
            standby_sleep_wakeup(lcd_device);
            rt_device_control(lcd_device, RTGRAPHIC_CTRL_POWERON, NULL);
        }
        draw(lcd_device, 0, 1);
        rt_thread_mdelay(1000);//Make sure to see what we drawn
    }
}

static void draw_idle_sleep_task(void *parameter)
{
    rt_device_t lcd_device = (rt_device_t) parameter;
    bool lcd_drawing;
    uint8_t idle_mode_on;

    while (loop_end())
    {
        rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_BUSY, &lcd_drawing);
        if (!lcd_drawing)
        {

            idle_mode_on = 1;
            rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_MODE, (void *)&idle_mode_on);

            standby_sleep_wakeup(lcd_device);

            idle_mode_on = 0;
            rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_MODE, (void *)&idle_mode_on);

        }
        draw(lcd_device, 0, 1);
        rt_thread_mdelay(1000);//Make sure to see what we drawn
    }
}

static void draw_suspend_resume_task(void *parameter)
{
    rt_device_t lcd_device = (rt_device_t) parameter;
    bool lcd_drawing;
    uint8_t idle_mode_on;

    while (loop_end())
    {
        rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_BUSY, &lcd_drawing);
        if (!lcd_drawing)
        {
            standby_sleep_wakeup(lcd_device);
        }
        draw(lcd_device, 0, 1);
        rt_thread_mdelay(1000);//Make sure to see what we drawn
    }
}

static rt_err_t run_case(rt_device_t lcd_device, uint32_t test_group, uint32_t loop_times)
{
    rt_kprintf("****run_case %d ****\n", test_group);

    rt_err_t result = rt_device_open(lcd_device, RT_DEVICE_OFLAG_RDWR);

    if (RT_EOK == result)
    {
        loop_cnt = loop_times;
        ancestor_tx_complete = lcd_device->tx_complete;
        p_async_draw_sema = rt_sem_create("lcdsem", 1, RT_IPC_FLAG_FIFO);
        rt_device_set_tx_complete(lcd_device, lcd_flush_done);
        if (rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_INFO, &lcd_info) == RT_EOK)
        {
            rt_kprintf("Lcd info w:%d, h%d, bits_per_pixel %d\r\n", lcd_info.width, lcd_info.height, lcd_info.bits_per_pixel);
        }

        uint16_t cf;
        if (16 == lcd_info.bits_per_pixel)
            cf = RTGRAPHIC_PIXEL_FORMAT_RGB565;
        else if (24 == lcd_info.bits_per_pixel)
            cf = RTGRAPHIC_PIXEL_FORMAT_RGB888;
        else
            RT_ASSERT(0);

        rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BUF_FORMAT, &cf);

#ifdef RT_USING_PM
        rt_pm_request(PM_SLEEP_MODE_IDLE);
        rt_pm_hw_device_start();
#endif  /* RT_USING_PM */

        if (0 == test_group)
        {
            rt_thread_t t_brightness = rt_thread_create("lcdt_a", brightness_set_task, lcd_device, 1024, RT_THREAD_PRIORITY_MIDDLE, 1);
            rt_thread_t t_draw_sync = rt_thread_create("lcdt_b", sync_draw_task, lcd_device, 1024, RT_THREAD_PRIORITY_MIDDLE, 1);
            rt_thread_t t_draw_async = rt_thread_create("lcdt_c", async_draw_task, lcd_device, 1024, RT_THREAD_PRIORITY_MIDDLE, 1);

            t_brightness->cleanup = task_cleanup;
            t_draw_sync->cleanup = task_cleanup;
            t_draw_async->cleanup = task_cleanup;
            running_task = 3;
            rt_thread_startup(t_brightness);
            rt_thread_startup(t_draw_sync);
            rt_thread_startup(t_draw_async);
        }
        else if (1 == test_group)
        {
            rt_thread_t t_draw_close_open = rt_thread_create("lcdt_d", draw_close_open_task, lcd_device, 1024, RT_THREAD_PRIORITY_MIDDLE, 1);

            t_draw_close_open->cleanup = task_cleanup;
            running_task = 1;
            rt_thread_startup(t_draw_close_open);
        }
        else if (2 == test_group)
        {
            rt_thread_t t_draw_onoff_sleep = rt_thread_create("lcdt_e", draw_onoff_sleep_task, lcd_device, 1024, RT_THREAD_PRIORITY_MIDDLE, 1);

            t_draw_onoff_sleep->cleanup = task_cleanup;
            running_task = 1;
            rt_thread_startup(t_draw_onoff_sleep);
        }
        else if (3 == test_group)
        {
            rt_thread_t t_draw_idle_sleep = rt_thread_create("lcdt_f", draw_idle_sleep_task, lcd_device, 1024, RT_THREAD_PRIORITY_MIDDLE, 1);

            t_draw_idle_sleep->cleanup = task_cleanup;
            running_task = 1;
            rt_thread_startup(t_draw_idle_sleep);
        }
        else if (4 == test_group)
        {
            rt_thread_t t_draw_supend_resume = rt_thread_create("lcdt_g", draw_suspend_resume_task, lcd_device, 1024, RT_THREAD_PRIORITY_MIDDLE, 1);

            t_draw_supend_resume->cleanup = task_cleanup;
            running_task = 1;
            rt_thread_startup(t_draw_supend_resume);
        }
        else
        {
            while (loop_end())
            {
                ;//Nothing todo
            }
        }

        //Waitting task exit
        while (running_task) rt_thread_mdelay(100);

        rt_thread_mdelay(1000);
        rt_device_set_tx_complete(lcd_device, ancestor_tx_complete);

        if (RT_EOK == result) rt_device_close(lcd_device);
        lcd_device = RT_NULL;

#ifdef RT_USING_PM
        rt_pm_release(PM_SLEEP_MODE_IDLE);
        rt_pm_hw_device_stop();
#endif  /* RT_USING_PM */

        rt_sem_delete(p_async_draw_sema);
        p_async_draw_sema = NULL;

    }
    else
    {
        rt_kprintf("Lcd open error!\n");
    }

    return result;
}

int main(void)
{
    rt_device_t lcd_device = rt_device_find("lcd");
    rt_err_t err;

    if (!lcd_device)
    {
        rt_kprintf("Can't find lcd\n");
        err = RT_EINVAL;
    }
    else
    {
        uint32_t all_passed_cnt = 0;
        while (1)
        {
            run_case(lcd_device, 0, 100);
            run_case(lcd_device, 1, 10);
            run_case(lcd_device, 2, 10);
            run_case(lcd_device, 3, 10);
            run_case(lcd_device, 4, 10);

            rt_kprintf("********all_passed_cnt=%d********\n", all_passed_cnt++);
        }
    }

    while (1)
    {
        rt_thread_mdelay(5000);
    }

    return RT_EOK;
}

