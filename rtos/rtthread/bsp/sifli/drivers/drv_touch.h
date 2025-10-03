/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_TOUCH_H__
#define __DRV_TOUCH_H__
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_io.h"

#ifdef __cplusplus
extern "C" {
#endif


#define IIC_RETRY_NUM 2

#define TOUCH_EVENT_UP      (0x01)
#define TOUCH_EVENT_DOWN    (0x02)
#define TOUCH_EVENT_MOVE    (0x03)
#define TOUCH_EVENT_NONE    (0x80)

struct touch_message
{
    rt_uint16_t x;
    rt_uint16_t y;
    rt_uint8_t event;
};
typedef struct touch_message *touch_msg_t;

struct touch_ops
{
    rt_err_t (* read_point)(touch_msg_t); //Retrun RT_EOK if more points is ready to read
    rt_err_t (* init)(void);
    rt_err_t (* deinit)(void);
};
typedef struct touch_ops *touch_ops_t;

struct touch_drivers
{
    rt_list_t       list;
    rt_bool_t (*probe)(void);
    rt_sem_t        isr_sem;
    touch_ops_t     ops;
    void           *user_data;
};
typedef struct touch_drivers *touch_drv_t;

extern void rt_touch_drivers_register(touch_drv_t drv);
extern rt_err_t rt_touch_irq_pin_attach(rt_uint32_t mode, void (*hdr)(void *args), void  *args);
extern rt_err_t rt_touch_irq_pin_detach(void);
extern rt_err_t rt_touch_irq_pin_enable(rt_uint32_t enabled);


#ifdef __cplusplus
}
#endif
#endif
