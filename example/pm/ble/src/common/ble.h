/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BLE_H__
#define __BLE_H__


#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

#include "bf0_ble_gap.h"
#include "bf0_sibles.h"
#include "bf0_sibles_advertising.h"



#ifdef __cplusplus
extern "C" {
#endif

#define BLE_APP_TIMEOUT_INTERVAL (5000)

typedef struct
{
    uint8_t is_power_on;
    uint8_t conn_idx;
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
    rt_mailbox_t mb_handle;
} app_env_t;

app_env_t *ble_app_get_env(void);
void ble_app_service_init(void);
void ble_app_service_init(void);
void ble_app_advertising_start(void);
void app_timeout_handler(void *parameter);

#ifdef __cplusplus
}
#endif
#endif /* __BLE_H__ */

