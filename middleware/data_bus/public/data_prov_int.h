/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DATA_PROV_INT_H
#define DATA_PROV_INT_H
#include <rtthread.h>
#include <stdbool.h>
#include <stdint.h>
#include "data_service.h"

/**
 ****************************************************************************************
* @addtogroup data_service_provider Data service Provider
* @ingroup middleware
* @brief Sifli Data service interface
* @{
****************************************************************************************
*/

/** Data service client */
typedef struct data_service_client_tag
{
    rt_list_t   node;             /*!<List node*/
    uint16_t    src_cid;
    uint8_t     conn_id;
    uint8_t     service_id;         /*!<Service ID*/
    uint8_t    *user_data;
    data_req_t *config;
} data_service_client_t;

/** Data service */
#define MAX_SVC_NAME_LEN        9
typedef struct data_service_tag
{
    bool active;                /*!<Service is active*/
    uint8_t id;                 /*!<Service ID*/
    uint8_t last_clnt;          /*!<Last client ID*/
    char name[MAX_SVC_NAME_LEN];/*!<Service name*/
    rt_list_t clients;          /*!<Service clients connected to this service */
    void **client_list;
    const struct data_service_config_tag *config;  /*!< service config */
    rt_list_t node;             /*!<List node */
    /// data fifo
    void *user_data;            /*!< User data of service */
} data_service_t;

/// @} data_service_provider
#endif

