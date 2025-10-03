/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HR_ALGO_PROCESS_H_
#define _HR_ALGO_PROCESS_H_


#include "board.h"
#include "sensor.h"
#include "hr_sensor_service.h"
#include "hr_algo.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t no_touch_num;
    uint8_t hr_timeout;
    uint8_t no_touch_one;
    uint8_t hr_timeout_one;
} hr_algo_ctrl_data_t;

void hrs_algo_process(uint8_t hr_type, hr_raw_data_t *hr_data, void *g_in);
uint8_t hrs_algo_postprocess(uint8_t hr_type, hrs_info_t *info);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_GOODIX_GH3011_H__

