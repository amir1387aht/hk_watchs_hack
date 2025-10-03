#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"
#include "dfs_posix.h"
#include "schrift.h"

/* Common functions for RT-Thread based platform -----------------------------------------------*/
/**
  * @brief  Initialize board default configuration.
  * @param  None
  * @retval None
  */
void HAL_MspInit(void)
{
    //__asm("B .");        /*For debugging purpose*/
    BSP_IO_Init();
}

/* User code start from here --------------------------------------------------------*/

#ifdef USE_8BIT_PIXEL
void lv_show_label(SFT *sftfont, char *text, uint32_t v_off, uint8_t font_size)
{
    lv_obj_t *canvas;
    int i = 0, x_off = 20;

    /*Test the rotation. It requires another buffer where the original image is stored.
     *So use previous canvas as image and rotate it to the new canvas*/
    int size = LV_IMG_BUF_SIZE_TRUE_COLOR(LV_HOR_RES, font_size * 2);
    uint8_t *buf = rt_calloc(1, size);

    /*Create a canvas and initialize its palette*/
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, buf, LV_HOR_RES, font_size * 2, LV_IMG_CF_TRUE_COLOR);

    lv_draw_img_dsc_t draw_dsc;
    lv_draw_img_dsc_init(&draw_dsc);
    draw_dsc.recolor = lv_color_white();
    lv_img_dsc_t img = {0};
    img.header.cf = LV_IMG_CF_ALPHA_8BIT;
    do
    {
        uint32_t letter = _lv_txt_encoded_next(text, &i); /*Character found, get it*/
        int width, height, y_offset;

        uint8_t *pixels;
        if (letter)
        {
            pixels = sft_get_glyph(sftfont, letter, font_size, &width, &height, &y_offset);
            if (pixels)
            {
                img.header.w = width;
                img.header.h = height;
                img.data_size = LV_IMG_BUF_SIZE_ALPHA_8BIT(width, font_size);
                img.data = pixels;
                lv_canvas_draw_img(canvas, x_off, font_size - height - y_offset, &img, &draw_dsc);
                lv_img_cache_invalidate_src(&img);
                x_off += width + 2;
            }
        }
    }
    while (text[i]);
    lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, 0, v_off);
    lv_obj_invalidate(canvas);
}
#else

void lv_show_label(SFT *sftfont, char *text, uint32_t v_off, uint8_t font_size)
{

    lv_obj_t *canvas;
    int i = 0, x_off = 20;


    int size = LV_IMG_BUF_SIZE_TRUE_COLOR(LV_HOR_RES, font_size * 2);
    uint8_t *buf = rt_calloc(1, size);

    /*Create a canvas of true color format*/
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, buf, LV_HOR_RES, font_size * 2, LV_IMG_CF_TRUE_COLOR);

    /*Font image is indexed 1bit color*/
    lv_draw_img_dsc_t draw_dsc;
    lv_draw_img_dsc_init(&draw_dsc);
    lv_img_dsc_t img = {0};


    // Set font size
    do
    {
        uint32_t letter = _lv_txt_encoded_next(text, &i); /*Character found, get it*/
        int width, height, y_offset;

        uint8_t *pixels_1bit;
        if (letter)
        {
            // Get 8bit/pixel font image for letter(must be unicode).
            pixels_1bit = sft_get_glyph(sftfont, letter, font_size, &width, &height, &y_offset);
            if (pixels_1bit)
            {
                img.data_size = LV_IMG_BUF_SIZE_INDEXED_1BIT(width, font_size);
                img.header.cf = LV_IMG_CF_INDEXED_1BIT;

                img.header.w = width;
                img.header.h = height;
                img.data = pixels_1bit;

                lv_img_buf_set_palette(&img, 1, lv_color_white());  // Set pallete color
                lv_img_buf_set_palette(&img, 0, lv_color_black());

                // Draw font on canvas
                lv_canvas_draw_img(canvas, x_off, font_size - height - y_offset, &img, &draw_dsc);
                lv_img_cache_invalidate_src(&img);                  // Clean image cache
                x_off += width + 2;
            }
        }
    }
    while (text[i]);
    lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, 0, v_off);
    lv_obj_invalidate(canvas);
}
#endif

#ifndef _WIN32
#ifndef FS_REGION_START_ADDR
    #error "Need to define file system start address!"
#endif
#include "drv_flash.h"
int mnt_init(void)
{
    char *name[2];

    rt_kprintf("===auto_mnt_init===\n");

    memset(name, 0, sizeof(name));

#ifdef RT_USING_SDIO
    //Waitting for SD Card detection done.
    int sd_state = mmcsd_wait_cd_changed(3000);
    if (MMCSD_HOST_PLUGED == sd_state)
    {
        rt_kprintf("SD-Card plug in\n");
        name[0] = "sd0";
    }
    else
    {
        rt_kprintf("No SD-Card detected, state: %d\n", sd_state);
    }
#endif /* RT_USING_SDIO */


    name[1] = "flash0";
    register_mtd_device(FS_REGION_START_ADDR, FS_REGION_SIZE, name[1]);

    for (uint32_t i = 0; i < sizeof(name) / sizeof(name[0]); i++)
    {
        if (NULL == name[i]) continue;

        if (dfs_mount(name[i], "/", "elm", 0, 0) == 0) // fs exist
        {
            rt_kprintf("mount fs on %s to root success\n", name[i]);
            break;
        }
        else
        {
            rt_kprintf("mount fs on %s to root fail\n", name[i]);
        }
    }

    return RT_EOK;
}
INIT_ENV_EXPORT(mnt_init);
#endif /* _WIN32 */

SFT *g_font;

/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
int main(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t ms;

    /* init littlevGL */
    ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK)
    {
        return ret;
    }
    lv_ex_data_pool_init();

#if 0
#ifdef _WIN32
    g_font = load_ttf("..\\disk\\test.ttf", 0);
#else
    g_font = load_ttf("./test.ttf", 0);
#endif
#else
    g_font = load_ttf((void *)g_ttf_font, g_ttf_font_size);
#endif
    /* Show label*/
    lv_show_label(g_font, "你好，思澈科技", 40, 30);
    lv_show_label(g_font, "abcdefghijklmn", 100, 24);
    lv_show_label(g_font, "OPQRSTUVWXYZ", 148, 16);
    lv_show_label(g_font, "0123456789", 180, 12);
    lv_show_label(g_font, "你好，思澈科技", 204, 30);

    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }
    return RT_EOK;

}


