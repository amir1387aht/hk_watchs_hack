#include "flashdb/fdb.h"
#include "gui_app_fwk.h"
#include "littlevgl2rtt.h"
#include "lv_ext_resource_manager.h"
#include "lvgl.h"
#include "lvsf.h"
#include "utils/gl_styles.h"
#include "utils/time_utils.h"
#include <rtdevice.h>
#include <stdio.h>
#include <time.h>
#include "gui_apps/music_player/music_player.h"
#include <stdio.h>
#include <string.h>
#include "perf/crown_pref.h"

#define APP_ID "MainScreen"

// Volume control command structure for deferred processing
typedef enum
{
    VOLUME_CMD_UP,
    VOLUME_CMD_DOWN
} volume_cmd_t;

// Add these new global variables for volume control
lv_obj_t *volume_slider = NULL;
lv_obj_t *volume_icon = NULL;
static lv_timer_t *volume_hide_timer = NULL;
static lv_timer_t *volume_update_timer = NULL;
static volatile volume_cmd_t pending_volume_cmd = VOLUME_CMD_UP;
static volatile bool volume_cmd_pending = false;
static uint8_t target_volume = 50; // Target volume to be set
static bool volume_visible = false;

// Existing global variables
LV_IMG_DECLARE(bg_blue)
LV_IMG_DECLARE(music_bg)
LV_IMG_DECLARE(music_icon)
LV_IMG_DECLARE(pause_icon)
LV_IMG_DECLARE(play_icon)
LV_IMG_DECLARE(next_icon)

lv_obj_t *timeText;
lv_obj_t *dateText;
lv_obj_t *music_title;
lv_obj_t *duration_title;
lv_obj_t *state_icon;
lv_obj_t *music_island_bg;

music_state_t cache_state = MUSIC_STATE_STOPPED;
struct tm tm_now;
static lv_timer_t *data_timer = NULL;

// Function declarations
void update_all_texts();
static void create_volume_slider(lv_obj_t *parent);
static void show_volume_slider(void);
static void hide_volume_slider(void);
static void volume_hide_timer_cb(lv_timer_t *timer);
static void process_volume_change(uint8_t new_volume);
static void volume_update_timer_cb(lv_timer_t *timer);
static void volume_slider_event_cb(lv_event_t *e);

static inline uint8_t clamp_u8(int v, int lo, int hi)
{
    if (v < lo)
        return (uint8_t)lo;
    if (v > hi)
        return (uint8_t)hi;
    return (uint8_t)v;
}

// Keep UI in sync from a single place
static void set_volume_ui(uint8_t v)
{
    if (!volume_slider || !volume_icon)
        return;

    // Set the slider value
    lv_slider_set_value(volume_slider, v, LV_ANIM_OFF);

    // CRITICAL: Force the indicator to update by invalidating it specifically
    lv_obj_invalidate(volume_slider);
    // Some LVGL versions need this to force indicator refresh
    lv_event_send(volume_slider, LV_EVENT_VALUE_CHANGED, NULL);

    // Update icon based on volume level
    if (v == 0)
    {
        lv_label_set_text(volume_icon, LV_SYMBOL_MUTE);
    }
    else if (v < 50)
    {
        lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
    }
    else
    {
        lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MAX);
    }
}

// Volume slider creation function
static void create_volume_slider(lv_obj_t *parent)
{
    volume_slider = lv_slider_create(lv_scr_act());

    if (lv_slider_get_mode(volume_slider) == LV_SLIDER_MODE_RANGE)
        lv_slider_set_left_value(volume_slider, 0, LV_ANIM_OFF);

    lv_obj_set_width(volume_slider, 12);
    lv_obj_set_height(volume_slider, 80);
    lv_obj_set_x(volume_slider, -15);
    lv_obj_set_y(volume_slider, 80);
    lv_obj_set_align(volume_slider, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xC5C5C5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(volume_slider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x4191FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(volume_slider, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_slider_set_mode(volume_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(volume_slider, 0, 100);

    uint8_t current_volume = clamp_u8(music_player_get_volume(), 0, 100);
    target_volume = current_volume;

    lv_slider_set_value(volume_slider, current_volume, LV_ANIM_OFF);

    // Add event callback for touch interaction
    lv_obj_add_event_cb(volume_slider, volume_slider_event_cb,
                        LV_EVENT_VALUE_CHANGED, NULL);

    // Volume icon below slider
    volume_icon = lv_label_create(volume_slider);
    lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_x(volume_icon, -15);
    lv_obj_set_y(volume_icon, 110);
    lv_obj_set_align(volume_icon, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_style_text_color(volume_icon, lv_color_hex(0x6BB6FF), 0);
    lv_obj_set_style_text_font(volume_icon, &lv_font_montserrat_20, 0);
    lv_obj_align(volume_icon, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Hide initially
    lv_obj_add_flag(volume_slider, LV_OBJ_FLAG_HIDDEN);
    volume_visible = false;
}

// Show volume slider
static void show_volume_slider(void)
{
    if (!volume_slider)
        return;

    // Reset/restart the hide timer
    if (volume_hide_timer)
    {
        lv_timer_del(volume_hide_timer);
        volume_hide_timer = NULL;
    }

    // Make visible if not already
    if (!volume_visible)
    {
        lv_obj_clear_flag(volume_slider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(volume_slider);
        volume_visible = true;
    }

    // Create new hide timer
    volume_hide_timer = lv_timer_create(volume_hide_timer_cb, 3000, NULL);
    lv_timer_set_repeat_count(volume_hide_timer, 1);
}

static void volume_slider_event_cb(lv_event_t *e)
{
    // Only process actual value changes, not our forced events
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);

        // Avoid feedback loop - only update if different from target
        if (v != target_volume)
        {
            uint8_t nv = clamp_u8(v, 0, 100);
            target_volume = nv;
            music_player_set_volume(nv);

            // Update icon only, slider is already at correct position
            if (nv == 0)
            {
                lv_label_set_text(volume_icon, LV_SYMBOL_MUTE);
            }
            else if (nv < 50)
            {
                lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
            }
            else
            {
                lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MAX);
            }
        }

        // Keep slider visible
        show_volume_slider();
    }
}

// Hide volume slider
static void hide_volume_slider(void)
{
    if (!volume_slider)
        return;

    lv_obj_add_flag(volume_slider, LV_OBJ_FLAG_HIDDEN);
    volume_visible = false;

    if (volume_hide_timer)
    {
        volume_hide_timer = NULL;
    }
}

// Timer callback to auto-hide volume slider
static void volume_hide_timer_cb(lv_timer_t *timer)
{
    hide_volume_slider();
}

// Timer callback to process volume changes in main thread
static void volume_update_timer_cb(lv_timer_t *timer)
{
    if (volume_cmd_pending)
    {
        // Get current volume
        uint8_t current_volume = target_volume;

        // Calculate new volume
        int new_vol;
        if (pending_volume_cmd == VOLUME_CMD_UP)
        {
            new_vol = current_volume + 5;
        }
        else
        {
            new_vol = current_volume - 5;
        }

        // Clamp to valid range
        uint8_t new_volume = clamp_u8(new_vol, 0, 100);

        // Update if changed
        if (new_volume != current_volume)
        {
            target_volume = new_volume;
            music_player_set_volume(new_volume);
            set_volume_ui(new_volume);
        }

        // Always show slider when scrolling
        show_volume_slider();

        // Clear the pending flag
        volume_cmd_pending = false;
    }
}

// ISR-safe scroll up function
static void on_scroll_up()
{
    pending_volume_cmd = VOLUME_CMD_UP;
    volume_cmd_pending = true;
}

// ISR-safe scroll down function
static void on_scroll_down()
{
    pending_volume_cmd = VOLUME_CMD_DOWN;
    volume_cmd_pending = true;
}

static void music_state_button_cb(lv_event_t *e)
{
    const music_info_t *info = music_player_get_info();

    if (info->state != MUSIC_STATE_STOPPED)
        music_player_toggle();

    update_all_texts();
}

void create_music_island(const music_info_t *info)
{
    lv_obj_t *screen = lv_scr_act();

    music_island_bg = lv_obj_create(screen);
    lv_obj_set_style_bg_color(music_island_bg, lv_color_black(), 0);
    lv_obj_set_style_radius(music_island_bg, 90, 0);
    lv_obj_align(music_island_bg, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_size(music_island_bg, 340, 53);

    lv_obj_t *music_icon_bg = lv_obj_create(music_island_bg);
    lv_obj_set_style_bg_color(music_icon_bg, lv_color_hex(0x424242), 0);
    lv_obj_set_style_radius(music_icon_bg, 90, 0);
    lv_obj_align(music_icon_bg, LV_ALIGN_LEFT_MID, 9, 0);
    lv_obj_set_size(music_icon_bg, 39, 39);

    lv_obj_t *music_icon_left = lv_img_create(music_icon_bg);
    lv_img_set_src(music_icon_left, LV_EXT_IMG_GET(music_icon));
    lv_obj_align(music_icon_left, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(music_icon_left, 30, 30);

    music_title = lv_label_create(music_island_bg);
    lv_obj_set_style_text_color(music_title, lv_color_white(), 0);
    lv_obj_align(music_title, LV_ALIGN_LEFT_MID, 52, -3);
    lv_obj_add_style(music_title, &small_font_style, 0);
    lv_obj_set_style_text_align(music_title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_size(music_title, 135, 28);

    lv_obj_t *music_state_bg = lv_btn_create(music_island_bg);
    lv_obj_set_style_bg_color(music_state_bg, lv_color_hex(0x424242), 0);
    lv_obj_set_style_radius(music_state_bg, 90, 0);
    lv_obj_align(music_state_bg, LV_ALIGN_RIGHT_MID, -9, 0);
    lv_obj_set_size(music_state_bg, 39, 39);

    lv_obj_add_event_cb(music_state_bg, music_state_button_cb, LV_EVENT_CLICKED,
                        NULL);

    state_icon = lv_img_create(music_state_bg);
    lv_img_set_src(state_icon, info->state == MUSIC_STATE_PLAYING
                                   ? LV_EXT_IMG_GET(play_icon)
                                   : LV_EXT_IMG_GET(pause_icon));
    lv_obj_align(state_icon, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(state_icon, 30, 30);
    lv_obj_add_event_cb(state_icon, music_state_button_cb, LV_EVENT_CLICKED,
                        NULL);

    duration_title = lv_label_create(music_island_bg);
    lv_obj_align(duration_title, LV_ALIGN_RIGHT_MID, -55, -5);
    lv_obj_add_style(duration_title, &small_font_style, 0);
    lv_obj_set_style_text_align(duration_title, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_size(duration_title, 92, 22);
    lv_obj_set_style_text_color(duration_title, lv_color_white(), 0);
}

void update_all_texts()
{
    time_utils_get_tm(&tm_now);

    const music_info_t *info = music_player_get_info();

    lv_label_set_text_fmt(timeText, "%02d:%02d", tm_now.tm_hour, tm_now.tm_min);
    lv_label_set_text(dateText, date_utils_get_str());

    if (info->state == MUSIC_STATE_STOPPED)
    {
        if (music_island_bg)
        {
            lv_obj_del(music_island_bg);
            music_island_bg = NULL;
            music_title = NULL;
            duration_title = NULL;
            state_icon = NULL;
        }
        return;
    }

    if (!music_island_bg)
    {
        create_music_island(info);
    }

    // Update music title
    lv_label_set_text(music_title, info->title);
    lv_label_set_text_fmt(duration_title, "%d:%02d/%d:%02d",
                          info->duration_sec / 60, info->duration_sec % 60,
                          info->position_sec / 60, info->position_sec % 60);

    if (strlen(info->title) > 15)
    {
        lv_label_set_long_mode(music_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_add_style(music_title, &moving_text_style, LV_PART_MAIN);
    }
    else
    {
        lv_label_set_long_mode(music_title, LV_LABEL_LONG_CLIP);
        lv_obj_remove_style(music_title, &moving_text_style, LV_PART_MAIN);
    }

    if (cache_state != info->state)
    {
        lv_img_set_src(state_icon, info->state == MUSIC_STATE_PLAYING
                                       ? LV_EXT_IMG_GET(pause_icon)
                                       : LV_EXT_IMG_GET(play_icon));
        cache_state = info->state;
    }
}

static void update_all_texts_timer_cb(lv_timer_t *timer)
{
    update_all_texts();
}

static void on_start(void)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_clean(screen);
    lv_obj_remove_style_all(screen);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);

    lv_obj_t *bg = lv_img_create(screen);
    lv_img_set_src(bg, LV_EXT_IMG_GET(bg_blue));
    lv_obj_align(bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(bg, 390, 450);

    timeText = lv_label_create(screen);
    lv_obj_set_style_text_color(timeText, lv_color_hex(0x444444), 0);
    lv_obj_align(timeText, LV_ALIGN_TOP_MID, 0, 85);
    lv_obj_add_style(timeText, &black_font_style, 0);
    lv_obj_set_style_text_align(timeText, LV_TEXT_ALIGN_CENTER, 0);

    dateText = lv_label_create(screen);
    lv_obj_set_style_text_color(dateText, lv_color_hex(0x444444), 0);
    lv_obj_align(dateText, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_add_style(dateText, &medium_font_style, 0);
    lv_obj_set_style_text_align(dateText, LV_TEXT_ALIGN_CENTER, 0);

    // Get current volume
    target_volume = clamp_u8(music_player_get_volume(), 0, 100);

    // Create the volume slider
    create_volume_slider(screen);

    // Show volume slider initially
    show_volume_slider();

    // Create timer for processing volume commands
    if (volume_update_timer)
    {
        lv_timer_del(volume_update_timer);
    }
    volume_update_timer = lv_timer_create(volume_update_timer_cb, 50, NULL);

    // Initialize music player
    music_player_set_track("Default Song T", "Demo Artist", 150);
    music_player_play();

    update_all_texts();

    // Create timer for updating time and music info
    if (data_timer)
    {
        lv_timer_del(data_timer);
        data_timer = NULL;
    }
    data_timer = lv_timer_create(update_all_texts_timer_cb, 1000, NULL);

    // Register crown callbacks
    crown_scroll_up_cb(on_scroll_up);
    crown_scroll_down_cb(on_scroll_down);
}

static void on_resume(void)
{
    // Re-register crown callbacks
    crown_scroll_up_cb(on_scroll_up);
    crown_scroll_down_cb(on_scroll_down);

    // Ensure volume slider is on top if visible
    if (volume_slider && volume_visible)
    {
        lv_obj_move_foreground(volume_slider);
    }
}

static void on_pause(void)
{
}

static void on_stop(void)
{
    // Clean up timers
    if (volume_hide_timer)
    {
        lv_timer_del(volume_hide_timer);
        volume_hide_timer = NULL;
    }

    if (volume_update_timer)
    {
        lv_timer_del(volume_update_timer);
        volume_update_timer = NULL;
    }

    if (data_timer)
    {
        lv_timer_del(data_timer);
        data_timer = NULL;
    }

    // Unregister crown callbacks
    crown_scroll_up_cb(NULL);
    crown_scroll_down_cb(NULL);
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

static int app_mainmenu(intent_t i)
{
    gui_app_regist_msg_handler(APP_ID, msg_handler);
    return 0;
}

BUILTIN_APP_EXPORT(LV_EXT_STR_ID(MainApp), NULL, APP_ID, app_mainmenu);