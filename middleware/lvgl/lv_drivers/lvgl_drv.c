/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "littlevgl2rtt.h"
#include "lvgl.h"
#include "board.h"

extern void lv_sifli_img_decoder(void);
extern lv_disp_drv_t *lv_lcd_init(const char *name);
extern void touch_init(void);
extern void keypad_init(void);
extern void wheel_init(void);
#if LV_USE_GPU
    extern void lv_gpu_init(lv_disp_drv_t *drv);
#endif

void gpu_lcd_init(const char *name)
{
    lv_disp_drv_t *disp = lv_lcd_init(name);

    lv_disp_drv_register(disp);
}

void lv_hal_init(const char *name)
{
    /* LCD Device Init */
    gpu_lcd_init(name);

    lv_sifli_img_decoder();

    /* littlevGL Input device interface */
    touch_init();

    // Init keyboard driver
    keypad_init();

    wheel_init();

}

