#include <rtthread.h>
#include <rtdevice.h>
#include "bf0_hal.h"
#include "drv_epic.h"
#include "drv_lcd.h"
#include "mem_section.h"

#define BUFFER_SIZE   (LCD_HOR_RES_MAX * LCD_VER_RES_MAX * 2)

// RGB565 宏定义
#define RGB888_TO_RGB565(r, g, b) \
    ((uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)))

// 分配显存空间
L2_NON_RET_BSS_SECT_BEGIN(epic_buffers)
ALIGN(64) static uint8_t buffer0[BUFFER_SIZE]; // 蓝色矩形
ALIGN(64) static uint8_t buffer1[BUFFER_SIZE]; // 红色矩形（带透明度）
ALIGN(64) static uint8_t buffer2[BUFFER_SIZE];
L2_NON_RET_BSS_SECT_END

void lcd_display_update(uint8_t *buffer)
{
    rt_kprintf("lcd_display...\n");
    rt_device_t lcd_dev = rt_device_find("lcd");
    if (!lcd_dev)
    {
        rt_kprintf("Failed to find LCD device\n");
        return;
    }

    if (rt_device_open(lcd_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Failed to open LCD device\n");
        return;
    }

    uint16_t buffer_format = RTGRAPHIC_PIXEL_FORMAT_RGB565;
    rt_device_control(lcd_dev, RTGRAPHIC_CTRL_SET_BUF_FORMAT, &buffer_format);

    struct rt_device_graphic_info info;
    rt_memset(&info, 0, sizeof(info));
    rt_device_control(lcd_dev, RTGRAPHIC_CTRL_GET_INFO, &info);

    rt_kprintf("LCD Info: Width=%d, Height=%d, BPP=%d\n", info.width, info.height, info.bits_per_pixel);

    struct rt_device_graphic_ops *ops = rt_graphix_ops(lcd_dev);
    if (ops && ops->draw_rect_async)
    {
        ops->set_window(0, 0, LCD_HOR_RES_MAX - 1, LCD_VER_RES_MAX - 1);
        ops->draw_rect_async((const char *)buffer, 0, 0, LCD_HOR_RES_MAX - 1, LCD_VER_RES_MAX - 1);
        rt_kprintf("draw_rect_async called\n");
    }
    else
    {
        rt_kprintf("draw_rect_async not available!\n");
    }
}

void epic_blend_layers(void)
{
    rt_kprintf("Starting EPIC blend demo...\n");

    // Layer 1: 蓝色
    EPIC_LayerConfigTypeDef fg_layer;
    HAL_EPIC_LayerConfigInit(&fg_layer);
    fg_layer.data = buffer0;
    fg_layer.color_mode = EPIC_COLOR_RGB565;
    fg_layer.width = 150;
    fg_layer.height = 100;
    fg_layer.x_offset = 50;
    fg_layer.y_offset = 50;
    fg_layer.total_width = LCD_HOR_RES_MAX;
    fg_layer.color_en = false;
    fg_layer.alpha = 128;
    fg_layer.ax_mode = ALPHA_BLEND_RGBCOLOR;
    HAL_EPIC_LayerSetDataOffset((EPIC_BlendingDataType *)&fg_layer, 150, 150);


    // Layer 2: 红色图层
    EPIC_LayerConfigTypeDef bg_layer;
    HAL_EPIC_LayerConfigInit(&bg_layer);
    bg_layer.data = buffer1;
    bg_layer.color_mode = EPIC_COLOR_RGB565;
    bg_layer.width = 150;
    bg_layer.height = 100;
    bg_layer.x_offset = 100;
    bg_layer.y_offset = 100;
    bg_layer.total_width = LCD_HOR_RES_MAX;
    bg_layer.color_en = false;
    bg_layer.alpha = 128;
    bg_layer.ax_mode = ALPHA_BLEND_RGBCOLOR;
    HAL_EPIC_LayerSetDataOffset((EPIC_BlendingDataType *)&bg_layer, 200, 200); //剪切，x，y为要剪切的起始坐标，对应layer左上角，注意x与y的坐标偏移应该是x_offset + x0


    EPIC_LayerConfigTypeDef input_layers[] = {fg_layer, bg_layer};
    uint8_t input_layer_num = sizeof(input_layers) / sizeof(EPIC_LayerConfigTypeDef);

    EPIC_LayerConfigTypeDef output_layer;
    HAL_EPIC_LayerConfigInit(&output_layer);
    output_layer.data = buffer2;
    output_layer.color_mode = EPIC_COLOR_RGB565;
    output_layer.width = LCD_HOR_RES_MAX;
    output_layer.height = LCD_VER_RES_MAX;
    output_layer.x_offset = 0;
    output_layer.y_offset = 0;
    output_layer.total_width = LCD_HOR_RES_MAX;
    output_layer.color_en = true;
    output_layer.alpha = 255;



    // 混叠
    rt_err_t ret = drv_epic_blend(input_layers, input_layer_num, &output_layer, NULL);
    if (ret != RT_EOK)
    {
        rt_kprintf("EPIC blend failed (%d)\n", ret);
    }
    else
    {
        rt_kprintf("EPIC blend succeeded\n");
        ret = drv_epic_wait_done();
        if (ret == RT_EOK)
        {
            lcd_display_update(buffer2);
        }
        else
        {
            rt_kprintf("Blend timeout or error (%d)\n", ret);
        }
    }
}

int epic_demo_init(void)
{
    rt_memset(buffer2, 0, BUFFER_SIZE);

    // 初始化 GPU，包括中断等资源
    drv_gpu_open();
    rt_kprintf("drv_gpu_open ok\n");

    EPIC_AreaTypeDef fill_area = {.x0 = 100, .y0 = 100, .x1 = 250, .y1 = 200};
    EPIC_AreaTypeDef dst_area = {.x0 = 0, .y0 = 0, .x1 = LCD_HOR_RES_MAX - 1, .y1 = LCD_VER_RES_MAX - 1};

    // 填充蓝色矩形到 buffer0
    uint32_t argb_color = 0xFF0000FF; // ARGB8888 蓝色
    rt_err_t ret = drv_epic_fill(
                       EPIC_COLOR_RGB565, buffer0, &dst_area, &fill_area, argb_color, 0, NULL, NULL, NULL
                   );
    if (ret != RT_EOK)
    {
        rt_kprintf("Fill blue rect failed\n");
        return -1;
    }


    // 填充红色矩形到 buffer1
    ret = drv_epic_fill(
              EPIC_COLOR_RGB565, buffer1, &dst_area, &fill_area, 0xFFFF0000, 0, NULL, NULL, NULL
          );
    if (ret != RT_EOK)
    {
        rt_kprintf("Fill red rect failed\n");
        return -1;
    }


    // 开始混合
    epic_blend_layers();

    return 0;
}


int main(int argc, char *argv[])
{
    rt_kprintf("Application started\n");
    epic_demo_init();
    while (1)
    {
        rt_thread_mdelay(1000);
    }
    return 0;
}