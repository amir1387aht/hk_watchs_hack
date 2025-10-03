/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SST25VFXX_MTD_H
#define SST25VFXX_MTD_H

#include <rtthread.h>
#include <drivers/spi.h>

rt_err_t sst25vfxx_mtd_init(const char *spi_device_name, rt_uint32_t block_start, rt_uint32_t block_end);

#endif

