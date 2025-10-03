/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __UI_DATASRV_H__
#define __UI_DATASRV_H__

#include "data_service_subscriber.h"

/**
 ****************************************************************************************
* @addtogroup ui_datac GUI Data service
* @ingroup data_service
* @brief Data service client for GUI application. It helps isolate GUI application thread from
*   data service running.
* @{
****************************************************************************************
*/

/**
    @brief Initialize GUI data service client module.
*/

#ifdef __cplusplus
extern "C" {
#endif

void ui_datac_init(void);

/**
    @brief Subscribe data service in GUI thread context.
    @param[in] handle data client handle
    @param[in] name Data service name
    @param[in] cbk Callback functions for data service.
    @param[in] user_data Callback function parameter. Service provide will call callback with it.
    @retval Pointer to message body.
    */
datac_handle_t ui_datac_subscribe(datac_handle_t handle, char *name, data_callback_t cbk, uint32_t user_data);

///@} ui_datac

#ifdef __cplusplus
}
#endif
#endif /*__UI_DATASRV_H__*/

