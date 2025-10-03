#include "rtconfig.h"
#include <time.h>
#include "lvsf.h"
#include <lv_ext_resource_manager.h>
#include <rtthread.h>
#include "gui_app_fwk.h"
#include "littlevgl2rtt.h"

LV_IMG_DECLARE(img_clock);

#define APP_ID "Watchface"

typedef enum
{
    STATE_DEINIT = 0,
    STATE_PAUSED,
    STATE_ACTIVE,
} watchface_state;

static void on_start(void)
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Watchface");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

static void on_resume(void)
{
}

static void on_pause(void)
{

}

static void on_stop(void)
{
    
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

static int app_main(intent_t i)
{
    gui_app_regist_msg_handler(APP_ID, msg_handler);
    return 0;
}

BUILTIN_APP_EXPORT(LV_EXT_STR_ID(WatchfaceString), LV_EXT_IMG_GET(img_clock), APP_ID, app_main);