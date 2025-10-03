/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"
#include "tc_utils.h"

#ifdef HAL_PMU_MODULE_ENABLED

/* Example Description:
 *
 * Config RTC to trigger alarm 10 second later
 * Enter hibernate, system would reboot 10 second later, boot log will be shown again
 *
 */

static RTC_HandleTypeDef RTC_Handler = {0};

static HAL_StatusTypeDef RTC_init()
{
    RTC_Handler.Instance = hwp_rtc;
    RTC_Handler.Init.DivAInt = 0x80;
    RTC_Handler.Init.DivAFrac = 0x0;
    RTC_Handler.Init.DivB = 0x100;
    RTC_Handler.Init.HourFormat = RTC_HOURFORMAT_24;

    if (HAL_RTC_Init(&RTC_Handler, 0) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    /* timer handle should be clear when delete timer. */
    return RT_EOK;
}

static void config_rtc_alarm(void)
{
    RTC_TimeTypeDef RTC_TimeStruct1 = {0};
    RTC_DateTypeDef RTC_DateStruct1 = {0};
    RTC_AlarmTypeDef sAlarm;

    RTC_DateStruct1.Year = 1999 - 1900;
    RTC_DateStruct1.Month = 12;
    RTC_DateStruct1.Date = 31;
    RTC_TimeStruct1.Hours = 23;
    RTC_TimeStruct1.Minutes = 50;
    RTC_TimeStruct1.Seconds = 10;

    if (RTC_init() != HAL_OK)
    {
        return LOG_I("RTC init error");
    }

    LOG_I("Set time (%04d-%02d-%02d %02d:%02d:%02d)",
          RTC_DateStruct1.Year + 1900, RTC_DateStruct1.Month, RTC_DateStruct1.Date,
          RTC_TimeStruct1.Hours, RTC_TimeStruct1.Minutes, RTC_TimeStruct1.Seconds);
    HAL_RTC_SetTime(&RTC_Handler, &RTC_TimeStruct1, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&RTC_Handler, &RTC_DateStruct1, RTC_FORMAT_BIN);

    /* set alarm at 10 second later */
    memset(&sAlarm, 0, sizeof(sAlarm));
    sAlarm.AlarmTime.Hours = 23;
    sAlarm.AlarmTime.Minutes = 50;

    sAlarm.AlarmTime.Seconds = 20;

    // Ignore Month,day and Weekday, and use high precison on subsecond match to avoid multiple interrupt in one alarm
    sAlarm.AlarmMask = RTC_ALRMDR_MSKD | RTC_ALRMDR_MSKM | RTC_ALRMDR_MSKWD | (10 << RTC_ALRMDR_MSKSS_Pos) ;

    LOG_I("Set Alarm at time (%02d:%02d:%02d)", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);

#if defined(SF32LB56X)
    /* workaround as XT32K cannot be used in hibernate */
    sAlarm.AlarmTime.Seconds = 13;
    HAL_PMU_LpCLockSelect(PMU_LPCLK_RC10);
    /* make sure clock has switched to RC10 */
    HAL_Delay_us(100);
    HAL_PMU_DisableXTAL32();
#endif /* SF32LB56X */
    HAL_RTC_SetAlarm(&RTC_Handler, &sAlarm, RTC_FORMAT_BIN);    // Year in Alarm is ignored.
}

static void testcase(int argc, char **argv)
{
    PMU_BootModeTypeDef boot_mode;
    uint32_t wakeup_src;
    HAL_StatusTypeDef err;

    err = HAL_PMU_CheckBootMode(&boot_mode, &wakeup_src);

    if (HAL_OK == err)
    {
        LOG_I("PMU boot mode: %d, wakeup src: %d", boot_mode, wakeup_src);
    }
    else
    {
        LOG_I("PMU boot mode check error");
    }

    config_rtc_alarm();
    /* Enable RTC wakeup */
    HAL_PMU_EnableRtcWakeup();
    LOG_I("Shutdown>>>>>>>>>");
    /* Enter hibernate mode, system would wakeup 10 second later */
    HAL_PMU_EnterHibernate();

    return;
}

#ifndef HAL_USING_HTOL
    UTEST_TC_EXPORT(testcase, "example_pmu", utest_tc_init, utest_tc_cleanup, 10);
#endif

#endif /* HAL_EPIC_MODULE_ENABLED */

