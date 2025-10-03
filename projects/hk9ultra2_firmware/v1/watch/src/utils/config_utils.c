#include "./config_utils.h"

void load_configurations(rt_device_t lcd)
{
    // Boot count
    int boot_count = kv_get_int("boot_count", 0);
    boot_count++;
    kv_set_int("boot_count", boot_count);
    rt_kprintf("Boot count: %d\n", boot_count);

    // Current date and time
    if (!datetime_load())
    {
        set_date(2025, 1, 1);
        set_time(0, 0, 0);
        datetime_save_current();
    }

    // Lcd brightness
    int lcd_brightness = kv_get_int("lcd_brightness", 100);

    if (lcd_brightness < 10 || lcd_brightness > 100)
    {
        lcd_brightness = 100;
        kv_set_int("lcd_brightness", lcd_brightness);
    }

    rt_device_control(lcd, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &lcd_brightness);
}