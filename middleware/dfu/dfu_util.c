/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <rtthread.h>
#include "board.h"

#include "dfu_internal.h"

/* 0, x ver; 1, y ver; 2, z ver.
  Covert to decimal digit, such as xxyyyzzz.
*/
uint32_t dfu_ctrl_convert_ver_2_digit(uint8_t *ver, uint8_t state)
{
    uint32_t num = atoi((const char *)ver);
    uint8_t state_off_map[] = {X_VER_OFFSET, Y_VER_OFFSET, Z_VER_OFFSET};
    if (state_off_map[state] != 0)
        for (uint32_t i = 0; i < state_off_map[state]; i++)
            num = num * 10;
    return num;
}

// Ver should be x.y.z
uint32_t dfu_ctrl_parser_SDK_version(uint8_t *str)
{
    uint8_t *ver = str;
    uint32_t ver_num = 0, digit = 0;
    uint8_t temp_digit[4] = {0};
    /* 0, x ver; 1, y ver; 2, z ver. */
    uint8_t state = 0;
    uint8_t state_map[3] = {X_VER_MAX_DIGIT, Y_VER_MAX_DIGIT, Z_VER_MAX_DIGIT};
    if (*ver++ != 'v')
        return ver_num;

    uint8_t *t_v = ver;
    while (*t_v != NULL)
    {
        if ((*t_v >= '0') && (*t_v <= '9'))

        {
            digit++;
        }
        else if (*t_v == '.' ||
                 (state = 2 && digit == state_map[state])) /*the end of z may not have .*/
        {
            memcpy(temp_digit, ver, digit);
            ver_num += dfu_ctrl_convert_ver_2_digit(temp_digit, state);
            state++;
            ver = t_v + 1;
            digit = 0;
        }
        else
            break;

        t_v++;
        if (digit > state_map[state])
            break;

        if (state > 2)
            break;
    }

    if (state == 3)
        return ver_num;
    else
        return 0;
}

uint32_t dfu_ctrl_get_current_HW_version(void)
{
    return __HAL_SYSCFG_GET_CID();
}

#define str_(s)  #s
#define str(s)   str_(s)

uint32_t dfu_ctrl_get_current_SDK_version(void)
{
#ifndef SIFLI_VERSION
    return 0xFFFFFFFF;
#else
    //return dfu_ctrl_parser_SDK_version((uint8_t *)str(SIFLI_REV));
    return SIFLI_VERSION;
#endif
}

/* 0 means failed; 1 means successful. */
__WEAK uint8_t dfu_ctrl_compare_FW_version(void)
{
    return 1;
}

dfu_image_header_int_t *dfu_img_get_img_header_by_img_id(dfu_ctrl_env_t *env, uint8_t img_id)
{
    dfu_dl_image_header_t *header = &env->prog.fw_context.code_img;
    if (env->mode == DFU_CTRL_NORMAL_MODE && env->ota_state.dfu_ID == DFU_ID_OTA_MANAGER)
    {
        header = (dfu_dl_image_header_t *)&env->ota_state.fw_context.code_img;
    }
    for (uint32_t i = 0; i < header->img_count; i++)
        if (header->img_header[i].img_id == img_id)
            return &header->img_header[i];
    return NULL;
}

dfu_image_header_int_t *dfu_img_get_img_header_by_img_id_ext(dfu_ctrl_ext_env_t *env, uint8_t img_id)
{
    dfu_dl_image_header_t *header = &env->prog.fw_context.code_img;
    for (uint32_t i = 0; i < header->img_count; i++)
        if (header->img_header[i].img_id == img_id)
            return &header->img_header[i];
    return NULL;
}

