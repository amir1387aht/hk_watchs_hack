/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"

#ifdef HAL_MAILBOX_MODULE_ENABLED

/* Example Description:
 *
 * 1. Demonstrate HPSYS mutex usage, lock success and lock fail
 * 2. Demonstrate LPSYS mutex usage, lock success and lock fail
 */

static MUTEX_LockCoreIdTypeDef lock_core_id_map[] =
{
    [CORE_ID_DEFAULT] = MUTEX_LOCK_CORE_INVALID,
    [CORE_ID_HCPU] = MUTEX_HCPU_LOCKED,
    [CORE_ID_LCPU] = MUTEX_LCPU_LOCKED,
    //[CORE_ID_BCPU] = MUTEX_BCPU_LOCKED,
};

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void test_mutex(MUTEX_CH_TypeDef *mutex)
{
    MUTEX_HandleTypeDef handle;
    MUTEX_LockCoreIdTypeDef core;
    MUTEX_LockCoreIdTypeDef exp_core_id;

    handle.Instance = mutex;

    /* lock first time, success */
    core = HAL_MAILBOX_Lock(&handle, 0);
    /* check whether succeed to lock */
    uassert_true_ret(core == MUTEX_UNLOCKED);
    /* lock second time, fail, it has been locked before */
    core = HAL_MAILBOX_Lock(&handle, 0);
    /* it's locked by current core */
    exp_core_id = lock_core_id_map[CORE_ID_CURRENT];
    LOG_I("core:%d,%d,%d,%x", core, exp_core_id, CORE_ID_CURRENT, mutex);
    uassert_true_ret(core == exp_core_id);

    /* unlock */
    HAL_MAILBOX_UnLock(&handle, 0);
    /* lock again, success */
    core = HAL_MAILBOX_Lock(&handle, 0);
    uassert_true_ret(core == MUTEX_UNLOCKED);
    core = HAL_MAILBOX_Lock(&handle, 0);
    LOG_I("core:%d,%d,%d,%x", core, exp_core_id, CORE_ID_CURRENT, mutex);
    exp_core_id = lock_core_id_map[CORE_ID_CURRENT];
    uassert_true_ret(core == exp_core_id);
    HAL_MAILBOX_UnLock(&handle, 0);
}

static void test_hmutex(void)
{
    /* test HPSYS mutex channel1 */
    test_mutex(HMUTEX_CH1);
    /* test HPSYS mutex channel2 */
    test_mutex(HMUTEX_CH2);
}

static void test_lmutex(void)
{
    /* test LPSYS mutex channel1 */
    test_mutex(LMUTEX_CH1);
}

static void example_mailbox_mutex(int argc, char **argv)
{
    test_hmutex();
    test_lmutex();
}

UTEST_TC_EXPORT(example_mailbox_mutex, "example_mailbox_mutex", NULL, NULL, 10);

#endif /*HAL_CRC_MODULE_ENABLED*/

