/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

#define ASSIGNED_VALUE 0x5AA5A5A5
#define ASSIGNED_VALUE_H 0x5AA5
#define ASSIGNED_VALUE_B 0xA5

static void testcase(int argc, char **argv)
{
    volatile uint32_t data_l;
    volatile uint16_t data_h;
    volatile uint8_t data_b;

    uint32_t value = (ASSIGNED_VALUE >> 1);
    uint16_t value_h = (uint16_t)(ASSIGNED_VALUE_H >> 1);
    uint8_t value_b = (uint8_t)(ASSIGNED_VALUE_B >> 1);
    rt_kprintf("value=%x, value_h=%x, value_b=%x\n", value, (uint32_t)value_h, (uint32_t)value_b);

#ifdef HAL_ATOMIC_FIX_ENABLED
    // Enable support for atomic instruction on PRO
    hwp_hpsys_cfg->SYS_RSVD |= 3;
#endif /*HAL_ATOMIC_FIX_ENABLED*/

    data_l = __LDREXW(&value);
    data_l = (ASSIGNED_VALUE >> 1);
    if (__STREXW(ASSIGNED_VALUE, &data_l) == 0)
        rt_kprintf("Write success\n");
    else
        rt_kprintf("Write fail\n");
    __utest_assert(data_l == ASSIGNED_VALUE, "STREXW value");

    data_h = __LDREXH(&value_h);
    if (__STREXH(ASSIGNED_VALUE_H, &data_h) == 0)
        rt_kprintf("Write half success\n");
    else
        rt_kprintf("Write half fail\n");
    __utest_assert(data_h == ASSIGNED_VALUE_H, "STREXH value");

    data_b = __LDREXB(&value_b);
    if (__STREXB(ASSIGNED_VALUE_H, &data_b) == 0)
        rt_kprintf("Write byte success\n");
    else
        rt_kprintf("Write byte fail\n");
    __utest_assert(data_b == ASSIGNED_VALUE_B, "STREXB value");

    rt_kprintf("value=%x, value_h=%x, value_b=%x\n", data_l, (uint32_t)data_h, (uint32_t)data_b);

    __CLREX();

}

UTEST_TC_EXPORT(testcase, "example_atomic", utest_tc_init, utest_tc_cleanup, 10);

/* Example to show to how to perform an exclusive read-modify-write using Load-Exclusive and Store-Exclusive instructions */
static void testcase_ex_readwrite(int argc, char **argv)
{
    uint32_t test_var = 0xabcd;
    uint32_t temp;

    temp = HAL_LOCK_Read32(&test_var);
    temp++;
    /* no exception occurs in between, write succeed */
    if (HAL_LOCK_Write32(&test_var, temp))
    {
        rt_kprintf("write1 succ:%x\n", test_var);
    }
    else
    {
        rt_kprintf("write1 fail\n");
        uassert_true_ret(false);
    }

    temp = HAL_LOCK_Read32(&test_var);
    temp++;
    /* wait for systick interrupt, it would result in write fail as exception occurs */
    rt_thread_mdelay(2);
    if (HAL_LOCK_Write32(&test_var, temp))
    {
        rt_kprintf("write2 succ:%x\n", test_var);
        uassert_true_ret(false);
    }
    else
    {
        rt_kprintf("write2 fail, try again\n");
    }

    /* try the entire read-modify-write sequence again */
    temp = HAL_LOCK_Read32(&test_var);
    temp++;
    if (HAL_LOCK_Write32(&test_var, temp))
    {
        rt_kprintf("write3 succ:%x\n", test_var);
    }
    else
    {
        rt_kprintf("write3 fail\n");
        uassert_true_ret(false);
    }

    temp++;
    /* write fails as no read precedes */
    if (HAL_LOCK_Write32(&test_var, temp))
    {
        rt_kprintf("write4 succ:%x\n", test_var);
        uassert_true_ret(false);
    }
    else
    {
        rt_kprintf("write4 fail\n");
    }
}
UTEST_TC_EXPORT(testcase_ex_readwrite, "example_ex_readwrite", NULL, NULL, 10);

