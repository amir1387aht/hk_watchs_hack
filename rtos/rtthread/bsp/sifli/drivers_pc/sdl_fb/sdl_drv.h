/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SDL_DRV_H
#define SDL_DRV_H

#include "rtconfig.h"
#define RTGUI_MOUSE_BUTTON_DOWN     0x01
#define RTGUI_MOUSE_BUTTON_LEFT     0x02
#define RTGUI_MOUSE_BUTTON_RIGHT    0x04
#define RTGUI_MOUSE_BUTTON_MIDDLE   0x08
#define RTGUI_MOUSE_BUTTON_UP       0x10

struct rtgui_event_mouse
{
    int x, y;
    rt_uint32_t button;
};

struct rtgui_event_kbd
{
    int mod;
    int key;
};
#ifdef LV_HOR_RES_MAX
    #define SDL_SCREEN_WIDTH    LV_HOR_RES_MAX
#else
    #define SDL_SCREEN_WIDTH    454
#endif
#ifdef LV_VER_RES_MAX
    #define SDL_SCREEN_HEIGHT   LV_VER_RES_MAX
#else
    #define SDL_SCREEN_HEIGHT   454
#endif

#define SDL_SCREEN_FORMAT   SDL_PIXELFORMAT_RGB565

void sdlfb_hw_init(void);

#endif

