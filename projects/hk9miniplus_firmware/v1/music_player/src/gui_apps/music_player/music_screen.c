#include "./music_player.h"

#define APP_ID "MusicScreen"

static void on_start(void)
{
    rt_kprintf("[MusicApp] on_start\n");

    lv_obj_t *volume_slider = lv_slider_create(lv_scr_act());

    if (lv_slider_get_mode(volume_slider) == LV_SLIDER_MODE_RANGE)
        lv_slider_set_left_value(volume_slider, 0, LV_ANIM_OFF);

    lv_obj_set_width(volume_slider, 12);
    lv_obj_set_height(volume_slider, 80);
    lv_obj_set_x(volume_slider, -15);
    lv_obj_set_y(volume_slider, 80);
    lv_obj_set_align(volume_slider, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xC5C5C5),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(volume_slider, 255,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xFFFFFF),
                              LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x4191FF),
                              LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(volume_slider, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_slider_set_mode(volume_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(volume_slider, 0, 100);

    //lv_slider_set_value(volume_slider, 50, LV_ANIM_OFF);
}
static void on_resume(void)
{
    rt_kprintf("[MusicApp] on_resume\n");
}
static void on_pause(void)
{
    rt_kprintf("[MusicApp] on_pause\n");
}
static void on_stop(void)
{
    rt_kprintf("[MusicApp] on_stop\n");
}

static void msg_handler(gui_app_msg_type_t msg, void *param)
{
    switch (msg)
    {
    case GUI_APP_MSG_ONSTART:
        on_start();
        break;
    case GUI_APP_MSG_ONRESUME:
        on_resume();
        break;
    case GUI_APP_MSG_ONPAUSE:
        on_pause();
        break;
    case GUI_APP_MSG_ONSTOP:
        on_stop();
        break;
    default:
        break;
    }
}

static int app_musicplayer(intent_t i)
{
    gui_app_regist_msg_handler(APP_ID, msg_handler);
    return 0;
}

BUILTIN_APP_EXPORT(LV_EXT_STR_ID(MusicPlayer), NULL, APP_ID, app_musicplayer);