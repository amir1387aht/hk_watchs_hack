/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MONITOR_H
#define MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_drv_conf.h"

#if USE_MONITOR

#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void monitor_init(void);
void monitor_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void monitor_flush2(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void monitor_update_title_hint(const char *s);
void ut_wait(size_t line, const char *hint);
int ut_continue(void);

/**********************
 *      MACROS
 **********************/

#endif /* USE_MONITOR */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MONITOR_H */

