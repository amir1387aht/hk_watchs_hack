/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdlib.h>
#include "utest.h"
#include "bf0_hal.h"
#include "tc_utils.h"

#ifdef HAL_RTC_MODULE_ENABLED

/*
    This example demo:
        1. Set RTC to 1999-12-31 23:59:59
        2. Wait for 3 seconds
        3. Read RTC, should be 2020-01-01 00:00:02
        4. Set Alarm at 00:00:05
        5. Alarm should trigger at 00:00:05
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

void RTC_IRQHandler(void)
{
    ENTER_INTERRUPT();
    HAL_RTC_IRQHandler(&RTC_Handler);
    LEAVE_INTERRUPT();
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)   // Weak symbol implement the interrupt of Alarm.
{
    LOG_I("Alarm interrupt\n");
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

static void testcase(int argc, char **argv)
{
    RTC_TimeTypeDef RTC_TimeStruct1 = {0};
    RTC_DateTypeDef RTC_DateStruct1 = {0};

    RTC_DateStruct1.Year = 1999 - 1900;
    RTC_DateStruct1.Month = 12;
    RTC_DateStruct1.Date = 31;
    RTC_TimeStruct1.Hours = 23;
    RTC_TimeStruct1.Minutes = 59;
    RTC_TimeStruct1.Seconds = 59;

    if (RTC_init() == HAL_OK)
    {
        LOG_I("Set time (%04d-%02d-%02d %02d:%02d:%02d)",
              RTC_DateStruct1.Year + 1900, RTC_DateStruct1.Month, RTC_DateStruct1.Date,
              RTC_TimeStruct1.Hours, RTC_TimeStruct1.Minutes, RTC_TimeStruct1.Seconds);
        HAL_RTC_SetTime(&RTC_Handler, &RTC_TimeStruct1, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&RTC_Handler, &RTC_DateStruct1, RTC_FORMAT_BIN);

        LOG_I("Wait 3s");
        rt_thread_mdelay(3000);

        HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct1, RTC_FORMAT_BIN);

        while (HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct1, RTC_FORMAT_BIN) == HAL_ERROR)
        {
            HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct1, RTC_FORMAT_BIN);
        };
        LOG_I("Get time (%04d-%02d-%02d %02d:%02d:%02d)",
              (RTC_DateStruct1.Year & RTC_CENTURY_BIT) ? RTC_DateStruct1.Year + 1900 : RTC_DateStruct1.Year + 2000,
              RTC_DateStruct1.Month, RTC_DateStruct1.Date,
              RTC_TimeStruct1.Hours, RTC_TimeStruct1.Minutes, RTC_TimeStruct1.Seconds);

    }

    // Set alarm at 00:00:05
    {
        RTC_AlarmTypeDef sAlarm;
        memset(&sAlarm, 0, sizeof(sAlarm));
        sAlarm.AlarmTime.Hours = 00;
        sAlarm.AlarmTime.Minutes = 00;
        sAlarm.AlarmTime.Seconds = 5;

        // Ignore Month,day and Weekday, and use high precison on subsecond match to avoid multiple interrupt in one alarm
        sAlarm.AlarmMask = RTC_ALRMDR_MSKD | RTC_ALRMDR_MSKM | RTC_ALRMDR_MSKWD | (10 << RTC_ALRMDR_MSKSS_Pos) ;

        HAL_NVIC_SetPriority(RTC_IRQn, 3, 0);    /* set the RTC priority */
        HAL_NVIC_EnableIRQ(RTC_IRQn);            /* enable the RTC global Interrupt */

        LOG_I("Set Alarm at time (%02d:%02d:%02d)", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
        HAL_RTC_SetAlarm(&RTC_Handler, &sAlarm, RTC_FORMAT_BIN);    // Year in Alarm is ignored.
        rt_thread_mdelay(6000);
    }
    return;
}

UTEST_TC_EXPORT(testcase, "example_rtc", utest_tc_init, utest_tc_cleanup, 10);
#endif

