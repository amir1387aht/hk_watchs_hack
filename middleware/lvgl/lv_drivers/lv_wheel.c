/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtthread.h"
#include "littlevgl2rtt.h"
#include "lvgl.h"
#include "lvsf.h"
#include "board.h"

static lv_indev_drv_t indev_drv;
static rt_device_t wheel_device = NULL;

static void input_read(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)   //(lv_indev_data_t *data)//(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data
{

    if (wheel_device)
    {
        int16_t wheel_data;

        if (2 == rt_device_read(wheel_device, 0, &wheel_data, 2))
        {
            data->enc_diff = wheel_data;

            wheel_do_event(data->enc_diff, data->state);
        }
    }

#if 0//def BSP_USING_LVGL_INPUT_AGENT
    {
        extern int lv_indev_agent_filter(lv_indev_drv_t *drv, lv_indev_data_t *data);
        lv_indev_agent_filter(indev_drv, data);
    }
#endif

}

static lv_indev_t *wheel_indev;

void wheel_init(void)
{
    /*Open wheel device*/
    RT_ASSERT(NULL == wheel_device);
    wheel_device = rt_device_find("wheel");
    if (wheel_device)
    {
        if (RT_EOK == rt_device_open(wheel_device, RT_DEVICE_FLAG_RDONLY))
        {
            //slv_indev_t *dev;
            lv_indev_drv_init(&indev_drv);

            indev_drv.type = LV_INDEV_TYPE_ENCODER;
            indev_drv.read_cb = input_read;
            wheel_indev = lv_indev_drv_register(&indev_drv);
        }
        else
        {
            wheel_device = NULL;
            wheel_indev = NULL;
        }
    }

#if 0//def BSP_USING_LVGL_INPUT_AGENT
    {
        extern void lv_indev_agent_init(lv_indev_drv_t *drv);
        lv_indev_agent_init(&wheel_indev->driver);
    }
#endif

}

lv_indev_t *wheel_get_indev_handler(void)
{
    return wheel_indev;
}

