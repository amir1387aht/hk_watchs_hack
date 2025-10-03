/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CPU_USAGE_PROFILER_H
#define CPU_USAGE_PROFILER_H
#include <stdint.h>
#include <stdbool.h>

/**
 ****************************************************************************************
* @addtogroup cpu_usage_profiler CPU Usage Profiler
* @ingroup middleware
* @brief Profiling CPU usage
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USING_CPU_USAGE_PROFILER
float cpu_get_usage(void);
uint32_t cpu_get_hw_us(void);
#elif !defined(LCD_SDL2)
#define cpu_get_usage()     0
#define cpu_get_hw_us()     0
#endif

/// @}  cpu_usage_profiler

#ifdef __cplusplus
}
#endif

/// @} file
#endif /* CPU_USAGE_PROFILER_H */

