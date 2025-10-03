/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"

#ifdef HAL_EFUSE_MODULE_ENABLED

/* Example Description:
 *
 * 1. read 4 bytes from bit offset 0, i.e. bit0~bit31 of bank0, print the value
 * 2. read 4 bytes from bit offset 768, i.e. bit0~bit31 of bank3, print the value
 *
 */

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(int argc, char **argv)
{
    uint32_t bit_offset;
    uint32_t val;
    int32_t r;

    bit_offset = 0;
    val = 0;

    //--------- read ----------
    r = HAL_EFUSE_Read(bit_offset, (uint8_t *)&val, 4);
    uassert_true_ret(4 == r);
    LOG_I("efuse read: off:%d,  value:0x%08X\n", bit_offset, val);

    bit_offset = 768;
    r = HAL_EFUSE_Read(bit_offset, (uint8_t *)&val, 4);
    uassert_true_ret(4 == r);
    LOG_I("efuse read: off:%d,  value:0x%08X\n", bit_offset, val);

#if 0
    //--------- write ----------
    r = HAL_EFUSE_Write(bit_offset, (uint8_t *)&val, 4);
    uassert_true_ret(4 == r);
    rt_kprintf("eefuse write:  off:%d,  value:0x%08X\n", argv[0], bit_offset, val);
#endif

}

UTEST_TC_EXPORT(testcase, "example_efuse", utest_tc_init, utest_tc_cleanup, 10);

#endif /* HAL_EZIP_MODULE_ENABLED */

