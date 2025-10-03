/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <rthw.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <rtdevice.h>

#include  <windows.h>
#include  <mmsystem.h>
#include  <conio.h>

struct rt_device _device;

static rt_err_t lcd_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_device_graphic_info *info;

    info = (struct rt_device_graphic_info *) args;
    info->bandwidth = 48000000;

    return RT_EOK;
}

void lcd_dummy_hw_init(void)
{

    _device.init = RT_NULL;
    _device.open = RT_NULL;
    _device.close = RT_NULL;
    _device.read = RT_NULL;
    _device.write = RT_NULL;
    _device.control = lcd_control;

    rt_device_register(&_device, "lcd", RT_DEVICE_FLAG_RDWR);
}

