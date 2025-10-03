/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CONTEXT_BACKUP_H
#define CONTEXT_BACKUP_H
#include <stdint.h>
#include <stdbool.h>

/**
 ****************************************************************************************
* @addtogroup context_backup Context Backup and Restore
* @ingroup middleware
* @brief Context Backup and Restore, context including heap and stack
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    /** start address */
    uint32_t start_addr;
    /** region length in byte */
    uint32_t len;
} cb_retained_region_t;

/** backup stack */
#define CB_BACKUP_STACK_MASK         ((uint8_t)1 << 0)
/** backup heap */
#define CB_BACKUP_HEAP_MASK          ((uint8_t)1 << 1)
/** backup static data */
#define CB_BACKUP_STATIC_DATA_MASK   ((uint8_t)1 << 2)
/** backup all */
#define CB_BACKUP_ALL_MASK    (CB_BACKUP_STACK_MASK | CB_BACKUP_HEAP_MASK | CB_BACKUP_STATIC_DATA_MASK)

#define CB_MAX_BACKUP_REGION_NUM  (4)

typedef struct
{
    /** start address of retention memory to backup data */
    uint32_t ret_mem_start_addr;
    /** retention memory size in byte */
    uint32_t ret_mem_size;
    /** indicate which data types need backup
     *
     * such as #CB_BACKUP_STACK_MASK, #CB_BACKUP_HEAP_MASK
     */
    uint8_t backup_mask;
    /** backup region number */
    uint8_t backup_region_num;
    /** backup region list */
    cb_retained_region_t backup_region_list[CB_MAX_BACKUP_REGION_NUM];
} cb_backup_param_t;

rt_err_t cb_init(cb_backup_param_t *param);
rt_err_t cb_deinit(void);
rt_err_t cb_save_context(void);
rt_err_t cb_restore_context(void);
void cb_get_stats(uint32_t *total, uint32_t *min_free);

/// @}  context_backup

#ifdef __cplusplus
}
#endif

/// @} file
#endif /* CONTEXT_BACKUP_H */

