/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TC_VGLITE_H__
#define __TC_VGLITE_H__


#include "stdint.h"
#include "stdbool.h"
#include "rtthread.h"
#include "utest.h"



#ifdef __cplusplus
extern "C" {
#endif

void tc_vglite_print_error(const char *func, size_t line, vg_lite_error_t err);

#define TC_VGLITE_CHECK_ERROR(Function)                     \
    error = Function;                                       \
    if (error > 0)                                          \
    {                                                       \
        tc_vglite_print_error(__FUNCTION__, __LINE__, error);   \
        uassert_true(false);                                \
        goto ErrorHandler;                                  \
    }

void tc_vg_send_data_to_lcd(uint8_t *data, uint32_t width, uint32_t height, uint16_t color_fmt);

rt_err_t tc_vglite_init(void);
rt_err_t tc_vglite_cleanup(void);

const char *tc_vglite_get_buf_fmt_str(vg_lite_buffer_format_t buf_format);

const char *tc_vglite_get_blend_mode_str(vg_lite_blend_t blend_mode);

const char *tc_vglite_get_filter_str(vg_lite_filter_t filter);

const char *tc_vglite_get_layout_str(vg_lite_buffer_layout_t layout);


#ifdef __cplusplus
}
#endif
#endif /* __TC_VGLITE_H__ */

