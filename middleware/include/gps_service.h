/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef GPS_SERVICE_H
#define GPS_SERVICE_H
#include <rtthread.h>
#include <stdbool.h>
#include <stdint.h>
#include "data_service_subscriber.h"

enum
{
    MSG_SERVICE_LOCATION_REQ             = (MSG_SERVICE_CUSTOM_ID_BEGIN + 0x20),
    MSG_SERVICE_LOCATION_RSP             = MSG_SERVICE_LOCATION_REQ | RSP_MSG_TYPE,
};

#define GPS_LOCATION_LEN            (3*8)

typedef struct
{
    double lati;
    double longi;
    double alti;
} custom_gps_data_table_t;

/// @}  file

#endif  /*GPS_SERVICE_H*/

