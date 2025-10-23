#include "./gl_styles.h"

lv_style_t black_font_style;
lv_style_t medium_font_style;
lv_style_t small_font_style;

lv_style_t moving_text_style;

void gs_init(void)
{
    // Black Font Initialization
    const lv_font_t *black_font = lvsf_get_font_by_name("SF_Pro_Display_Black", 72);

    if (black_font == NULL)
    {
        LV_LOG_ERROR("Font SF_Pro_Display_Black not found");
        return;
    }

    lv_style_init(&black_font_style);
    lv_style_set_text_font(&black_font_style, black_font);

    // Medium Font Initialization
    const lv_font_t *medium_font = lvsf_get_font_by_name("SF_Pro_Display_Medium", 28);

    if (medium_font == NULL)
    {
        LV_LOG_ERROR("Font SF_Pro_Display_Medium not found");
        return;
    }

    lv_style_init(&medium_font_style);
    lv_style_set_text_font(&medium_font_style, medium_font);

    // Small Font Initialization
    const lv_font_t *small_font =
        lvsf_get_font_by_name("SF_Pro_Display_Medium", 20);

    if (small_font == NULL)
    {
        LV_LOG_ERROR("Font SF_Pro_Display_Medium not found");
        return;
    }

    lv_style_init(&small_font_style);
    lv_style_set_text_font(&small_font_style, small_font);

    // Moving Text Style
    lv_style_init(&moving_text_style);
    lv_style_set_anim_speed(&moving_text_style, 13);
}