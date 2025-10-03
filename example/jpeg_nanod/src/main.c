/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

#ifdef BSP_USING_TOUCHD
    #include "drv_touch.h"
#endif
#include "mem_section.h"

/*
    Initallize pinmux
*/
void HAL_MspInit(void)
{
    // __asm("B .");        /*For debugging purpose*/
    BSP_IO_Init();
}

static void draw_lcd(const uint8_t *p_framebuffer, uint32_t w, uint32_t h, uint16_t format)
{
    /*
        Open LCD Device and get LCD infomation
    */
    rt_device_t lcd_device = (rt_device_t) rt_device_find("lcd");
    if (rt_device_open(lcd_device, RT_DEVICE_OFLAG_RDWR) == RT_EOK)
    {
        struct rt_device_graphic_info info;
        if (rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_INFO, &info) == RT_EOK)
        {
            rt_kprintf("Lcd info w:%d, h%d, bits_per_pixel %d, draw_align:%d\r\n",
                       info.width, info.height, info.bits_per_pixel, info.draw_align);
        }
    }
    else
    {
        rt_kprintf("Lcd open error!\n");
        return;
    }

    /*Flush framebuffer to LCD*/
    rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BUF_FORMAT, &format);
    rt_graphix_ops(lcd_device)->set_window(0, 0, w - 1, h - 1);
    rt_graphix_ops(lcd_device)->draw_rect((const char *)p_framebuffer, 0, 0, w - 1, h - 1);

    /* Set LCD backlight brightness level */
    uint8_t brightness = 100;
    rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &brightness);

}

ALIGN(64)
uint8_t jpeg_data[] =
{
#include "100x100_jpeg.dat"
};

#ifdef USING_JPEG_NANODEC
extern int jpeg_nanod_decode2(uint8_t *byteStrmStart,
                              uint32_t streamTotalLen,
                              const char *out_cf,
                              uint8_t *out_buf,
                              uint32_t out_buf_len);

extern     int jpeg_nanod_decode_init(void);
extern     int jpeg_nanod_decode_get_dimension(uint8_t *byteStrmStart, uint32_t streamTotalLen, uint32_t *width, uint32_t *height);
extern int jpeg_nanod_decode_deinit(void);
#endif /*USING_JPEG_NANODEC*/

int main(void)
{
#ifdef USING_JPEG_NANODEC
    const char *decode_format = "RGB565"; //"ARGB8888"; //
    int ret;
    uint32_t width, height;
    uint8_t *out_buf;
    uint32_t jpeg_data_len, out_buf_size;

    jpeg_data_len = sizeof(jpeg_data);
    jpeg_nanod_decode_init();
    ret = jpeg_nanod_decode_get_dimension((uint8_t *)&jpeg_data[0], jpeg_data_len, &width, &height);

    if (0 == ret)
    {
        rt_kprintf("w=%d, h=%d \n", width, height);

        out_buf_size = width * height * 2;
        out_buf = rt_malloc(out_buf_size);

        ret = jpeg_nanod_decode2((uint8_t *)&jpeg_data[0], jpeg_data_len, decode_format, out_buf, out_buf_size);

        if (jpeg_data_len == ret)
        {
            draw_lcd(out_buf, width, height, RTGRAPHIC_PIXEL_FORMAT_RGB565);
        }
        else
        {
            rt_kprintf("decode fail, err=%d \n", ret);
        }

        rt_free(out_buf);
    }
    else
    {
        rt_kprintf("get dimension fail, err=%d \n", ret);
    }

    jpeg_nanod_decode_deinit();
#endif /*USING_JPEG_NANODEC*/

    while (1)
    {
        rt_thread_mdelay(5000);
        rt_kprintf("__main loop__\r\n");
    }

    return RT_EOK;
}

