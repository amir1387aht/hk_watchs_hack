/*
 * COPYRIGHT (C) 2018, Real-Thread Information Technology Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-5-30     Bernard      the first version
 */

#ifndef __MTD_DHARA_H__
#define __MTD_DHARA_H__

#include "map.h"

#define RT_MTD_DHARA_DEVICE(device)   ((struct rt_mtd_dhara_device*)(device))

typedef struct
{
    struct dhara_map map;
    uint8_t *page_buffer;
    struct dhara_nand nand;
} dhara_ctx_t;

struct rt_mtd_dhara_device
{
    struct rt_device parent;

    uint8_t gc_ratio;
    rt_mtd_nand_t mtd_nand;
    dhara_ctx_t dhara_ctx;
};

rt_err_t rt_mtd_dhara_register_device(const char *name, uint8_t gc_ratio, rt_mtd_nand_t mtd_nand);


#endif /* __MTD_DHARA_H__ */
