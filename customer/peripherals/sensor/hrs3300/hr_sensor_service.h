/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HR_SENSOR_SERVICE_H_
#define _HR_SENSOR_SERVICE_H_

#include "board.h"
#include "sensor.h"

#if defined (HR_ALGO_USING_GSENSOR_DATA)
    #include "gsensor_service.h"
#endif

#define HR_PERIOD_TIMER     40
#define HR_ALGO_PEROID      1000

typedef enum
{
    HEALTH_ALG_NOT_OPEN = 0x01,
    HEALTH_NO_TOUCH,
    HEALTH_PPG_LEN_TOO_SHORT,
    HEALTH_HR_READY,
    HEALTH_ALG_TIMEOUT,
    HEALTH_SETTLE
} hr_msg_code_t;

typedef enum
{
    MSG_ALG_NOT_OPEN = 0x01,
    MSG_NO_TOUCH = 0x02,
    MSG_PPG_LEN_TOO_SHORT = 0x03,
    MSG_HR_READY = 0x04,
    MSG_ALG_TIMEOUT = 0x05,
    MSG_SETTLE = 0x06
} hrs3300_msg_code_t;

typedef enum
{
    MSG_BP_ALG_NOT_OPEN = 0x01,
    MSG_BP_NO_TOUCH = 0x02,
    MSG_BP_PPG_LEN_TOO_SHORT = 0x03,
    MSG_BP_READY = 0x04,
    MSG_BP_ALG_TIMEOUT = 0x05,
    MSG_BP_SETTLE = 0x06
} hrs3300_bp_msg_code_t;

typedef struct
{
    hrs3300_msg_code_t alg_status;
    uint32_t           data_cnt;
    uint8_t            hr_result;
    uint8_t            hr_result_qual; // ericy add20170111
    bool               object_flg;
    uint32_t           timestamp;
} hr_algo_result_t;

typedef struct
{
    hrs3300_bp_msg_code_t bp_alg_status;
    uint8_t            sbp;
    uint8_t            dbp;
    uint32_t           data_cnt;
    uint8_t            hr_result; //20170614 ericy
    bool               object_flg;
    uint32_t           timestamp;
} bp_algo_result_t;

typedef struct
{
    uint8_t hr_id;
    uint8_t type;
} hr_sensor_info_t;

/* hr device structure */
struct hr_sensor_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
};

typedef struct
{
    uint16_t hrm_raw;
    uint16_t alg_raw;
} hr_raw_data_t;

#endif  // SENSOR_GOODIX_GH3011_H__

