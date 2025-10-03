/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __VIBRATOR_FY_H__
#define __VIBRATOR_FY_H__



#include <rtthread.h>
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * This function will prepare to use vibrator
 *
 * @return failed if < 0
 */

#ifdef __cplusplus
extern "C" {
#endif

rt_err_t vibrator_open();

/**
 * This function will output PWM wave to conctorl vibrator, this function will re
 *
 * @param vibrator on time in msenconds
 * @param vibrator off time in msenconds
 * @param output on/off times
 *
 * @return failed if < 0
 */
rt_size_t vibrator_write(rt_uint32_t on_time_ms, rt_uint32_t off_time_ms, rt_uint32_t repeat_times);

/**
 * This function will stop vibrator
 *
 * @return failed if < 0
 */
rt_err_t vibrator_close();


#ifdef __cplusplus
}
#endif
#endif /* __VIBRATOR_FY_H__ */
