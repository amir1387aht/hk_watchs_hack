/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LOG_TAG
    #define DBG_SECTION_NAME               "drv"
#else
    #define DBG_SECTION_NAME               LOG_TAG
#endif /* LOG_TAG */

#ifndef DBG_LEVEL
    #ifdef DRV_DEBUG
        #define DBG_LEVEL                      DBG_LOG
    #else
        #define DBG_LEVEL                      DBG_INFO
    #endif /* DRV_DEBUG */
#endif

#include <rtdbg.h>

