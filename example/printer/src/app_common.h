/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_COMMON_H
#define APP_COMMON_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <stdlib.h>

#include "bf0_ble_gap.h"
#include "bf0_sibles.h"
#include "bf0_sibles_advertising.h"
#include "ble_connection_manager.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t is_power_on;
    uint8_t conn_idx;
    uint8_t is_bg_adv_on;
    uint8_t is_timer_on;
    struct
    {
        bd_addr_t peer_addr;
        uint16_t mtu;
        uint16_t conn_interval;
        uint8_t peer_addr_type;
    } conn_para;
    struct
    {
        sibles_hdl srv_handle;
        uint32_t data;
        uint8_t is_config_on;
    } data;
    rt_timer_t time_handle;
    rt_timer_t write_time_handle;
#ifdef LXT_DISABLE
    rt_timer_t rc10k_time_handle;
#endif
    rt_mailbox_t mb_handle;
} app_env_t;

typedef struct
{
    U8 version_x;
    U8 version_y;
    U16 version_z;
} version_t;

extern app_env_t *ble_app_get_env(void);

extern void spi_task(void *parameter);
extern void pwm_task(void *parameter);
extern void adc_task(void *parameter);
extern void wheel_task(void *parameter);
extern void spp_init(void);
extern version_t bt_get_current_SDK_version(void);

extern void ble_init(void);
extern void ble_app_service_init(void);
extern void ble_app_advertising_start(void);
#define APP_TIMEOUT_INTERVAL (5000)

#ifdef __cplusplus
}
#endif
#endif