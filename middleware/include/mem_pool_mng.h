/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MEM_POOL_MNG_H
#define MEM_POOL_MNG_H
#include "rtthread.h"
#include <stdint.h>
#include <stdbool.h>
#include "section.h"

/**
 ****************************************************************************************
* @addtogroup mem_pool_mng Memory Pool Manager
* @ingroup middleware
* @brief Memory Pool Manager
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MEM_POOL_L1_NON_RET_SIZE
#define MEM_POOL_L1_NON_RET_SIZE 0
#endif

#ifndef MEM_POOL_L2_NON_RET_SIZE
#define MEM_POOL_L2_NON_RET_SIZE 0
#endif
#ifndef MEM_POOL_L2_RET_SIZE
#define MEM_POOL_L2_RET_SIZE 0
#endif
#ifndef MEM_POOL_L2_CACHE_NON_RET_SIZE
#define MEM_POOL_L2_CACHE_NON_RET_SIZE 0
#endif
#ifndef MEM_POOL_L2_CACHE_RET_SIZE
#define MEM_POOL_L2_CACHE_RET_SIZE 0
#endif

/** memory pool id
 *
 * Fallback policy:
 *    L1_NON_RET -> L1_RET
 *    L2_CACHE_RET -> L2_RET -> L1_RET
 *    L2_CACHE_NON_RET -> L2_NON_RET -> L1_NON_RET
 *
 */
typedef enum
{
    MEM_POOL_L1_NON_RET = 0,
    MEM_POOL_L1_RET     = 1,
    MEM_POOL_CACHE_L1_NON_RET = 2,
    MEM_POOL_CACHE_L1_RET = 3,
    MEM_POOL_L2_NON_RET = 4,
    MEM_POOL_L2_RET     = 5,
    MEM_POOL_L2_CACHE_NON_RET = 6,
    MEM_POOL_L2_CACHE_RET     = 7,
    MEM_POOL_NUM
} mem_pool_id_t;

typedef struct
{
    void         *start_addr;                 /**< pool start address and size */
    uint32_t     pool_size;                  /**< pool size */
    uint32_t     available_size;             /**< available size */
    uint32_t     max_used_size;              /**< maximum allocated size */
} mem_pool_info_t;

void *mem_pool_alloc(mem_pool_id_t pool_id, size_t size);

void *mem_pool_realloc(void *p, size_t new_size);

void mem_pool_free(void *p);

void *mem_pool_calloc(mem_pool_id_t pool_id, size_t count, size_t size);

bool mem_pool_get_info(mem_pool_id_t pool_id, mem_pool_info_t *info);

/// @}  mem_pool_mng

#ifdef __cplusplus
}
#endif

/// @} file
#endif

