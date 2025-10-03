/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

#ifdef BSP_USING_LCD_FRAMEBUFFER
    #include "drv_lcd_fb.h"
    #include "drv_epic.h"
#endif /* BSP_USING_LCD_FRAMEBUFFER */
#include "mem_section.h"

static uint8_t get_pixel_size(uint16_t color_format)
{
    uint8_t pixel_size;

    if (RTGRAPHIC_PIXEL_FORMAT_RGB565 == color_format)
    {
        pixel_size = 2;
    }
    else if (RTGRAPHIC_PIXEL_FORMAT_RGB888 == color_format)
    {
        pixel_size = 3;
    }
    else
    {
        pixel_size = 0;
    }

    return pixel_size;
}

#ifdef BSP_USING_EPIC
static void fill_color(uint8_t *buf, uint32_t width, uint32_t height,
                       uint16_t cf, uint32_t ARGB8888)
{
    rt_err_t err;
    EPIC_AreaTypeDef epic_dst_area;
    uint32_t dst_cf;

    if (RTGRAPHIC_PIXEL_FORMAT_RGB565 == cf)
    {
        dst_cf = EPIC_INPUT_RGB565;
    }
    else if (RTGRAPHIC_PIXEL_FORMAT_RGB888 == cf)
    {
        dst_cf = EPIC_INPUT_RGB888;
    }
    else
    {
        RT_ASSERT(0);
    }

    epic_dst_area.x0 = 0;
    epic_dst_area.y0 = 0;
    epic_dst_area.x1 = width - 1;
    epic_dst_area.y1 = height - 1;

    err = drv_epic_fill(dst_cf, (uint8_t *)buf,
                        &epic_dst_area, &epic_dst_area, ARGB8888,
                        0, (uint8_t *)NULL, NULL, NULL);

    RT_ASSERT(RT_EOK == err);
    drv_epic_wait_done();

}
#endif /* BSP_USING_EPIC */

#if defined(BSP_USING_LCD) && defined(BSP_USING_LCD_FRAMEBUFFER)
//#define L1_FRAMEBUFFER_ONLY

#define FB_COLOR_FORMAT RTGRAPHIC_PIXEL_FORMAT_RGB565
#define FB_PIXEL_BYTES  2

#ifdef L1_FRAMEBUFFER_ONLY
#define L1_FB_WIDTH  LCD_HOR_RES_MAX
#define L1_FB_HEIGHT (LCD_VER_RES_MAX)
#define L1_FB_TOTAL_BYTES (L1_FB_WIDTH * L1_FB_HEIGHT *FB_PIXEL_BYTES)
/*
    Define L1 framebuffer for rendering
*/
L1_NON_RET_BSS_SECT_BEGIN(frambuf)
L1_NON_RET_BSS_SECT(frambuf, ALIGN(64) static uint8_t l1_framebuffer1[L1_FB_TOTAL_BYTES]);
L1_NON_RET_BSS_SECT_END
static void dummy_func(void)
{
}

static int l1_only_test(uint32_t test_seconds, uint32_t rendering_period)
{

    lcd_fb_desc_t fb_dsc =
    {
        .area.x0 = 0,
        .area.y0 = 0,
        .area.x1 = L1_FB_WIDTH - 1,
        .area.y1 = L1_FB_HEIGHT - 1,
        .p_data = (uint8_t *) &l1_framebuffer1[0],
        .cmpr_rate = 0,
        .line_bytes = L1_FB_WIDTH * FB_PIXEL_BYTES,
        .format = FB_COLOR_FORMAT,
    };

    /*
        Check macro values
    */
    RT_ASSERT(get_pixel_size(FB_COLOR_FORMAT) == FB_PIXEL_BYTES);

    uint8_t color = 0;
    drv_lcd_fb_init("lcd");
    drv_lcd_fb_set(&fb_dsc);
    write_fb_cbk cb = NULL;

    uint32_t test_start_ms = rt_tick_get_millisecond();

    while (rt_tick_get_millisecond() - test_start_ms < test_seconds * 1000)
    {
        LCD_AreaDef src_area;
        src_area.x0 = 0;
        src_area.x1 = L1_FB_WIDTH - 1;
        src_area.y0 = 0;
        src_area.y1 = L1_FB_HEIGHT - 1;

        //rt_kprintf("L2 framebuffer addr=0x%x, w=%d, h=%d, size=%d(Bytes)\n", fb_dsc.p_data, L2_FB_WIDTH, L2_FB_HEIGHT, L2_FB_TOTAL_BYTES);

        uint8_t *p_render_fb = (uint8_t *)&l1_framebuffer1[0];
        uint32_t start_ms = rt_tick_get_millisecond();
        for (src_area.y0 = 0; src_area.y0 < L1_FB_HEIGHT;)
        {
            uint8_t last_one;

            src_area.y1 = L1_FB_HEIGHT - 1;
            drv_lcd_fb_get_write_area(&src_area, 100);

            if (L1_FB_HEIGHT - 1 == src_area.y1)
                last_one = 1;
            else
                last_one = 0;

            uint32_t height = src_area.y1 - src_area.y0 + 1;

            /*Fill framebuffer*/
            //rt_kprintf("Fill framebuffer addr=0x%x, y0~y1=%d~%d, size=%d(Bytes)\n", p_render_fb, src_area.y0, src_area.y1, L1_FB_TOTAL_BYTES);
            if (0 == color) fill_color((uint8_t *)p_render_fb, L1_FB_WIDTH, height, FB_COLOR_FORMAT, 0xFFFF0000); //Fill RED color
            if (1 == color) fill_color((uint8_t *)p_render_fb, L1_FB_WIDTH, height, FB_COLOR_FORMAT, 0xFF00FF00); //Fill GREEN color
            if (2 == color) fill_color((uint8_t *)p_render_fb, L1_FB_WIDTH, height, FB_COLOR_FORMAT, 0xFF0000FF); //Fill BLUE color
            if (3 == color) fill_color((uint8_t *)p_render_fb, L1_FB_WIDTH, height, FB_COLOR_FORMAT, 0xFFFFFFFF); //Fill WHITE color
            if (4 == color) fill_color((uint8_t *)p_render_fb, L1_FB_WIDTH, height, FB_COLOR_FORMAT, 0xFFFFFF00); //Fill YELLOW color

            if (last_one)
            {
                uint32_t cur_ms = rt_tick_get_millisecond();
                if (cur_ms - start_ms <= rendering_period)
                    rt_thread_delay(rt_tick_from_millisecond(rendering_period - (cur_ms - start_ms)));
                else
                    rt_kprintf("Rendering period:%dms, expect: %dms\n", cur_ms - start_ms, rendering_period);
            }

            drv_lcd_fb_write_send(&src_area, &src_area, (uint8_t *)p_render_fb, cb, last_one);

            src_area.y0 += height;
            p_render_fb += L1_FB_WIDTH * height * FB_PIXEL_BYTES;
        }

        color = (color + 1) % 5;
    }

    return RT_EOK;
}

#else /*!L1_FRAMEBUFFER_ONLY*/
#define L1_FB_WIDTH  LCD_HOR_RES_MAX
#define L1_FB_HEIGHT (LCD_VER_RES_MAX/4)
#define L1_FB_TOTAL_BYTES (L1_FB_WIDTH * L1_FB_HEIGHT *FB_PIXEL_BYTES)

#define L2_FB_WIDTH  LCD_HOR_RES_MAX
#define L2_FB_HEIGHT LCD_VER_RES_MAX
#define L2_FB_TOTAL_BYTES (L2_FB_WIDTH * L2_FB_HEIGHT *FB_PIXEL_BYTES)

/*
    Define L1 framebuffer for rendering
*/
L1_NON_RET_BSS_SECT_BEGIN(frambuf)
L1_NON_RET_BSS_SECT(frambuf, ALIGN(64) static uint8_t l1_framebuffer1[L1_FB_TOTAL_BYTES]);
L1_NON_RET_BSS_SECT(frambuf, ALIGN(64) static uint8_t l1_framebuffer2[L1_FB_TOTAL_BYTES]);
L1_NON_RET_BSS_SECT_END

/*
    Define L2 framebuffer for flushing LCD
*/
L2_NON_RET_BSS_SECT_BEGIN(frambuf)
L2_NON_RET_BSS_SECT(frambuf, ALIGN(64) static uint8_t l2_framebuffer1[L2_FB_TOTAL_BYTES]);
L2_NON_RET_BSS_SECT(frambuf, ALIGN(64) static uint8_t l2_framebuffer2[L2_FB_TOTAL_BYTES]);
L2_NON_RET_BSS_SECT_END

static void dummy_func(void)
{
}

static struct rt_semaphore done_sema;
static void lcd_flush_done(lcd_fb_desc_t *fb_desc)
{
    rt_sem_release(&done_sema);
}

static int l1_and_l2_test(uint32_t test_seconds, uint32_t l1_framebuf_cnt, uint32_t l2_framebuf_cnt, uint32_t rendering_period)
{

    lcd_fb_desc_t fb_dsc =
    {
        .area.x0 = 0,
        .area.y0 = 0,
        .area.x1 = L2_FB_WIDTH - 1,
        .area.y1 = L2_FB_HEIGHT - 1,
        .p_data = (uint8_t *) &l2_framebuffer1[0],
        .cmpr_rate = 0,
        .line_bytes = L2_FB_WIDTH * FB_PIXEL_BYTES,
        .format = FB_COLOR_FORMAT,
    };

    /*
        Check macro values
    */
    RT_ASSERT(get_pixel_size(FB_COLOR_FORMAT) == FB_PIXEL_BYTES);

    uint8_t color = 0;
    drv_lcd_fb_init("lcd");
    drv_lcd_fb_set(&fb_dsc);

    rt_sem_init(&done_sema, "donesem", 1, RT_IPC_FLAG_FIFO);

    uint8_t *p_render_fb = (uint8_t *)&l1_framebuffer1[0];

    uint32_t test_start_ms = rt_tick_get_millisecond();

    while (rt_tick_get_millisecond() - test_start_ms < test_seconds * 1000)
    {
        LCD_AreaDef src_area;
        src_area.x0 = 0;
        src_area.x1 = L2_FB_WIDTH - 1;

        //rt_kprintf("L2 framebuffer addr=0x%x, w=%d, h=%d, size=%d(Bytes)\n", fb_dsc.p_data, L2_FB_WIDTH, L2_FB_HEIGHT, L2_FB_TOTAL_BYTES);

        /*Fill framebuffer*/
        //rt_kprintf("Fill framebuffer addr=0x%x, y0~y1=%d~%d, size=%d(Bytes)\n", p_render_fb, src_area.y0, src_area.y1, L1_FB_TOTAL_BYTES);
        uint32_t v = 0xFFFF00FF; //Fill MAGENTA color
        if (0 == color) v = 0xFFFF0000; //Fill RED color
        else if (1 == color) v = 0xFF00FF00; //Fill GREEN color
        else if (2 == color) v = 0xFF0000FF; //Fill BLUE color
        else if (3 == color) v = 0xFFFFFFFF; //Fill WHITE color
        else if (4 == color) v = 0xFFFFFF00; //Fill YELLOW color

        /*Waitting done*/
        rt_sem_take(&done_sema, RT_WAITING_FOREVER);
        fill_color((uint8_t *)&l1_framebuffer1[0], L1_FB_WIDTH, L1_FB_HEIGHT, FB_COLOR_FORMAT, v);
        if (2 == l1_framebuf_cnt) fill_color((uint8_t *)&l1_framebuffer2[0], L1_FB_WIDTH, L1_FB_HEIGHT, FB_COLOR_FORMAT, v);

        uint32_t start_ms = rt_tick_get_millisecond();
        for (src_area.y0 = 0; src_area.y0 < L2_FB_HEIGHT;  src_area.y0 += L1_FB_HEIGHT)
        {
            uint8_t last_one;
            write_fb_cbk cb = NULL;

            if ((src_area.y0 + L1_FB_HEIGHT) >= L2_FB_HEIGHT)
            {
                src_area.y1 = L2_FB_HEIGHT - 1;
                last_one = 1;
            }
            else
            {
                src_area.y1 = src_area.y0 + L1_FB_HEIGHT - 1;
                last_one = 0;
            }

            if (last_one)
            {
                //rt_thread_delay(((uint32_t)rand())%rendering_period);
                cb = lcd_flush_done;
            }
            else
            {
                cb = NULL;
            }

            drv_lcd_fb_write_send(&src_area, &src_area, (uint8_t *)p_render_fb, cb, last_one);
            if (2 == l1_framebuf_cnt)
            {
                if (p_render_fb == (uint8_t *)&l1_framebuffer1[0])
                    p_render_fb = (uint8_t *)&l1_framebuffer2[0];
                else
                    p_render_fb = (uint8_t *)&l1_framebuffer1[0];
            }
        }

        color = (color + 1) % 5;

        if (2 == l2_framebuf_cnt)
        {
            if (rand() & 1) //Select framebuffer randomly
                fb_dsc.p_data = (uint8_t *)&l2_framebuffer2[0];
            else
                fb_dsc.p_data = (uint8_t *)&l2_framebuffer1[0];
            drv_lcd_fb_set(&fb_dsc);
        }

    }

    drv_lcd_fb_deinit();
    rt_sem_detach(&done_sema);

    return RT_EOK;
}
#endif /* L1_FRAMEBUFFER_ONLY */

#endif

int main(void)
{

#ifdef L1_FRAMEBUFFER_ONLY
    l1_only_test(10/*test seconds*/, 18 /*render period*/);
#else
    l1_and_l2_test(10/*test seconds*/, 1 /*l1 fb num*/, 1 /*L2 fb num*/, 18 /*render period*/);
    l1_and_l2_test(10/*test seconds*/, 1 /*l1 fb num*/, 2 /*L2 fb num*/, 18 /*render period*/);
    l1_and_l2_test(10/*test seconds*/, 2 /*l1 fb num*/, 1 /*L2 fb num*/, 18 /*render period*/);
    l1_and_l2_test(10/*test seconds*/, 2 /*l1 fb num*/, 2 /*L2 fb num*/, 18 /*render period*/);
#endif /* L1_FRAMEBUFFER_ONLY */

    return RT_EOK;
}

