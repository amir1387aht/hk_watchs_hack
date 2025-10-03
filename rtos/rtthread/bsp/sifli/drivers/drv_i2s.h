#ifndef __DRV_I2S_H__
#define __DRV_I2S_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <drv_common.h>

void bf0_i2s_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size);

#endif