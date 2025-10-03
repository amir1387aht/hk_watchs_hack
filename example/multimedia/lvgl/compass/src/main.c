#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"
#include "lv_compass.h"
#include "stdio.h"
#include "string.h"
#include "drv_gpio.h"
#include "sensor_memsic_mmc56x3.h"
#include "math.h"

struct rt_sensor_data mmc56x3;
static float last_smoothed_angle = 0.0f;
static bool first_measurement = true;
static const float EMA_ALPHA = 0.2f;

rt_device_t mmc56x3_dev;
// Initialize hardware I/O
static void board_io_init(void)
{
    HAL_PIN_Set(PAD_PA40, I2C2_SCL, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA39, I2C2_SDA, PIN_PULLUP, 1);
}

// Initialize the sensor
static void sensors_init(struct rt_sensor_config *cfg)
{
    cfg->intf.dev_name = "i2c2";
    rt_hw_mmc56x3_init("mmc56x3", cfg);
}

// Open and configure the sensor device
static void open_and_config_devices(rt_device_t *mmc56x3_dev)
{

    *mmc56x3_dev = rt_device_find("mag_mmc56x3");
    if (*mmc56x3_dev == RT_NULL)
        rt_kprintf("Can't find device:%s\n", "mmc56x3");
    else
    {
        rt_err_t ret = rt_device_open(*mmc56x3_dev, RT_DEVICE_FLAG_RDONLY);
        if (ret != RT_EOK)
            rt_kprintf("open device failed! err: %d\n", ret);
    }
}


// Normalize the Angle to -180 to 180°
static float normalize_angle(float angle)
{
    angle = fmod(angle, 360.0f);
    if (angle > 180.0f) angle -= 360.0f;
    if (angle < -180.0f) angle += 360.0f;
    return angle;
}

static float sensors_loop(rt_device_t mmc56x3_dev)
{
    rt_size_t res;

    res = rt_device_read(mmc56x3_dev, 0, &mmc56x3, 1);

    float single = atan2f(mmc56x3.data.mag.y, mmc56x3.data.mag.x) * 180 / 3.1415926;

    //Handle the first measurement
    if (first_measurement)
    {
        last_smoothed_angle = single;
        first_measurement = false;
        return single;
    }

    //  Calculate the difference from the last angle (considering 180° boundary issue)
    float angle_diff = single - last_smoothed_angle;

    // Normalize the difference to -180~180°
    if (angle_diff > 180.0f) angle_diff -= 360.0f;
    if (angle_diff < -180.0f) angle_diff += 360.0f;

    // Apply smoothing (smooth the angle difference)
    float smoothed_diff = EMA_ALPHA * angle_diff + (1 - EMA_ALPHA) * 0.0f;

    // Calculate the smoothed angle (based on the last angle and the difference)
    float smoothed_angle = last_smoothed_angle + smoothed_diff;

    // Ensure the smoothed angle is still within -180~180° range
    smoothed_angle = normalize_angle(smoothed_angle);
    // Update historical value

    last_smoothed_angle = smoothed_angle;
    //rt_kprintf("single:%f\n", smoothed_angle);
    return smoothed_angle;
}


void my_timer(lv_timer_t *timer)
{
    float sensor_angle;

    volatile  rt_tick_t start_tick, end_tick;
    sensor_angle = sensors_loop(mmc56x3_dev);

    pointer_rotate_cb(sensor_angle * 10 - 1600);
}


int main(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t ms;

    struct rt_sensor_config cfg;
    board_io_init();
    sensors_init(&cfg);
    open_and_config_devices(&mmc56x3_dev);

    /* init littlevGL */
    ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK)
    {
        return ret;
    }
    lv_ex_data_pool_init();


    example_compass();
    rt_kprintf("The version used is lvgl v8\n");
    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }
    return RT_EOK;

}
