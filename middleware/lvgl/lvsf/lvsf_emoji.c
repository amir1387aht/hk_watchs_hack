/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtthread.h"
#include "lv_ext_resource_manager.h"
#ifdef RT_USING_DFS
    #include <dfs.h>
    #include <dfs_file.h>
#endif

#if defined (SOLUTION_WATCH) && defined (EMOJI_SUPPORT)
    #include "images_emoji_header.h"
#endif

#if 0 //ifdef RT_USING_DFS
#define EMOJI_RES_PATH    "/ex/resource/"
#define EMOJI_RES_SUFFIX  ".bin"

//emoji filename format:  emoji_xxx. where xxx is letter's unicode.
char emoji_file_name[60];

static inline void *lv_get_emoji_by_unicode(uint32_t u_letter)
{
    RT_ASSERT(strlen(EMOJI_RES_PATH EMOJI_RES_SUFFIX "emoji_") + 8 < 60);
    sprintf(emoji_file_name, "%semoji_%x%s", EMOJI_RES_PATH, u_letter, EMOJI_RES_SUFFIX);
    //sprintf(emoji_file_name, "%s%s%s", "/ex/resource/", "clock_bg", ".bin");

    struct stat buf;
    if (RT_EOK == dfs_file_stat(emoji_file_name, &buf))
    {
        return &emoji_file_name;
    }

    return NULL;
}
#else
void *lv_get_emoji_by_unicode(uint32_t u_letter)
{
#define GET_EMOJI_INFO(_id) \
    case 0x##_id: \
    { \
    return (void *)&emoji_##_id;\
    }

    switch (u_letter)
    {
#ifndef SOLUTION_WATCH
        /*
        GET_EMOJI_INFO(1f302)
        GET_EMOJI_INFO(1f392)
        GET_EMOJI_INFO(1f393)
        GET_EMOJI_INFO(1f39a)
        GET_EMOJI_INFO(2639)
        GET_EMOJI_INFO(263a)
        GET_EMOJI_INFO(26d1)
        */
#else /*for SOLUTION_WATCH*/
#ifdef EMOJI_SUPPORT
#include "emoji_info.h"
#endif
#endif
    default:
        return NULL;
    }
}
#endif

