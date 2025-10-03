#ifndef GL_STYLES_H
#define GL_STYLES_H

#include <rtthread.h>
#include <rtdevice.h>
#include "lvsf_ft_reg.h"
#include "lv_ext_resource_manager.h"
#include "littlevgl2rtt.h"
#include "lvgl.h"
#include "lvsf.h"

#if LV_USE_FREETYPE
    #include "lv_freetype.h"
#endif

extern lv_style_t black_font_style;
extern lv_style_t medium_font_style;
extern lv_style_t small_font_style;

extern lv_style_t moving_text_style;

// Initialize all global styles
void gs_init(void);

#endif // GL_STYLES_H
