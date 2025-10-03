#include <rtdevice.h>
#include "app_mem.h"
#include "drv_lcd.h"
#include "gui_app_fwk.h"
#include "littlevgl2rtt.h"
#include "log.h"
#include "lv_ex_data.h"
#include "lv_ext_resource_manager.h"
#include "lv_freetype.h"
#include "perf/btn_perf.h"
#include "flashdb/fdb.h"
#include "utils/datetime_utils.h"
#include "utils/config_utils.h"
#include "utils/custom_trans_anim.h"
#include "perf/vibrator_perf.h"
#include "perf/crown_pref.h"
#include "utils/gl_styles.h"

#define APP_WATCH_GUI_TASK_STACK_SIZE 16 * 1024
#define LCD_DEVICE_NAME "lcd"

static struct rt_thread watch_thread;
static rt_device_t lcd_device;

ALIGN(RT_ALIGN_SIZE)
static uint8_t watch_thread_stack[APP_WATCH_GUI_TASK_STACK_SIZE];

#define WATCHFACE_APP_NAME "Watchface"
#define MENU_APP_NAME "MainMenu"

#ifdef BSP_KEY1_ACTIVE_HIGH
    #define DOWN_BUTTON_ACTIVE_POL 1
#else
    #define DOWN_BUTTON_ACTIVE_POL 0
#endif

#ifdef BSP_KEY2_ACTIVE_HIGH
    #define CROWN_BUTTON_ACTIVE_POL 1
#else
    #define CROWN_BUTTON_ACTIVE_POL 0
#endif

#define BSP_KEY3_PIN 25
#define LEFT_BUTTON_ACTIVE_POL 1

static void on_down_btn_callback(button_action_t event)
{
    if (event == BUTTON_PRESSED)
    {
        if (gui_app_is_actived(WATCHFACE_APP_NAME))
        {
            gui_app_run(MENU_APP_NAME);
        }
        else if (gui_app_is_actived(MENU_APP_NAME))
        {
            gui_app_run(WATCHFACE_APP_NAME);
        }
        else
        {
            gui_app_goback();
        }
    }
}

static void on_left_btn_callback(button_action_t event)
{
    if (event == BUTTON_PRESSED)
    {
        rt_kprintf("Button Left pressed");
    }
}

static void on_crown_btn_callback(button_action_t event)
{
    if (event == BUTTON_PRESSED)
    {
        if (gui_app_is_actived(WATCHFACE_APP_NAME))
        {
            gui_app_run(MENU_APP_NAME);
        }
        else if (gui_app_is_actived(MENU_APP_NAME))
        {
            gui_app_run(WATCHFACE_APP_NAME);
        }
        else
        {
            gui_app_goback();
        }
    }
}

static void app_watch_entry()
{
    lcd_device = rt_device_find(LCD_DEVICE_NAME);
    rt_err_t r = littlevgl2rtt_init(LCD_DEVICE_NAME);
    RT_ASSERT(RT_EOK == r);

    vibrator_buzz(200);

    lv_ex_data_pool_init();
    resource_init();
    lv_freetype_open_font(true);

    int down_btn_id = perf_button_register(BSP_KEY1_PIN, DOWN_BUTTON_ACTIVE_POL);
    perf_button_add_trigger(down_btn_id, on_down_btn_callback);

    int crown_btn_id = perf_button_register(BSP_KEY2_PIN, CROWN_BUTTON_ACTIVE_POL);
    perf_button_add_trigger(crown_btn_id, on_crown_btn_callback);

    int left_btn_id = perf_button_register(BSP_KEY3_PIN, LEFT_BUTTON_ACTIVE_POL);
    perf_button_add_trigger(left_btn_id, on_left_btn_callback);

    crown_init();
    gs_init();
    load_configurations(lcd_device);

    gui_app_init();
    gui_app_run(WATCHFACE_APP_NAME);

    lv_disp_trig_activity(NULL);
    lvsf_gesture_init(lv_layer_top());

    int ms_delay = 0;

    while (1)
    {
        ms_delay = lv_timer_handler();

        if (ms_delay > 0)
            rt_thread_mdelay(ms_delay);
    }
}

int app_watch_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_thread_t thread = RT_NULL;

    ret = rt_thread_init(&watch_thread, "app_watch", app_watch_entry, RT_NULL,
                         watch_thread_stack, APP_WATCH_GUI_TASK_STACK_SIZE,
                         RT_THREAD_PRIORITY_MIDDLE, RT_THREAD_TICK_DEFAULT);
    if (RT_EOK != ret)
        return RT_ERROR;

    rt_thread_startup(&watch_thread);

    return RT_EOK;
}

INIT_APP_EXPORT(app_watch_init);