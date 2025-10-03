/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DFU_PORT_SRV_H
#define __DFU_PORT_SRV_H

#include "data_service.h"

typedef enum
{
    BLE_DFU_SEND_DATA = MSG_SERVICE_CUSTOM_ID_BEGIN,

    /* Response. */
    BLE_DFU_SEND_DATA_RSP = BLE_DFU_SEND_DATA | RSP_MSG_TYPE,

    BLE_DFU_REBOOT_AFTER_DISCONNECT = BLE_DFU_SEND_DATA + 1,
    BLE_DFU_REBOOT_AFTER_DISCONNECT_RSP = BLE_DFU_REBOOT_AFTER_DISCONNECT | RSP_MSG_TYPE,

    BLE_DFU_REBOOT_DISCONNECT = BLE_DFU_REBOOT_AFTER_DISCONNECT + 1,
    BLE_DFU_REBOOT_DISCONNECT_RSP = BLE_DFU_REBOOT_DISCONNECT | RSP_MSG_TYPE,
} ble_dfu_service_message_t;

typedef struct
{
    uint8_t event;
    uint16_t len;
    uint8_t data[0];
} ble_dfu_service_data_t;

typedef struct
{
    uint8_t handle;         /**< Handle for the transmission channel. */
    uint8_t cate_id;        /**< CategoryID for different user. */
    uint16_t len;           /**< Data length. */
    uint8_t data[0];          /**< Transmission data. */
} ble_dfu_service_send_data_t;

#endif // __DFU_PORT_SRV_H

