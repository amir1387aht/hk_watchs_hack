/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <math.h>
#include <string.h>
#include "stdlib.h"
#include "board.h"
#include "gh3011_example.h"

#define DRV_DEBUG
#define LOG_TAG              "drv.hbd"
#include <drv_log.h>


#ifdef HR_USING_GH3011

extern void hal_gh30x_pin_set(uint8_t en);

int init_gh3011_sensor(void)
{
    int res =  gh30x_module_init();
    if (res == 1)
        return 0;

    return 1;
}

int open_gh3011(void)
{
    gh30x_module_start(2);
    return 0;
}

int close_gh3011(void)
{
    gh30x_module_stop();
    return 0;
}

#define DRV_GH3011_TEST

#ifdef DRV_GH3011_TEST
#include <string.h>

int cmd_hbd(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "-open") == 0)
        {
            int res = gh30x_module_init();
            LOG_I("Initial GH3011 %d\n", res);
        }
        else if (strcmp(argv[1], "-hb") == 0)
        {
            gh30x_module_start(2);
            LOG_I("start HB\n");
        }
        else if (strcmp(argv[1], "-spo") == 0)
        {
            gh30x_module_start(7);
            LOG_I("start spo2\n");
        }
        else if (strcmp(argv[1], "-hb2") == 0)
        {
            gh30x_module_start_without_adt(2);
            LOG_I("start HB\n");
        }
        else if (strcmp(argv[1], "-spo2") == 0)
        {
            gh30x_module_start_without_adt(7);
            LOG_I("start spo2\n");
        }
        else if (strcmp(argv[1], "-stop") == 0)
        {
            gh30x_module_stop();
            LOG_I("stop gh3011\n");
        }
        else
        {
            LOG_I("Invalid parameter\n");
        }
    }
    else
    {
        LOG_I("Invalid parameter\n");
    }
    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_hbd, __cmd_hbd, Test driver gh3011);

#endif //DRV_GH3011_TEST


#endif  // HR_USING_GH3011
/********END OF FILE********* (C) COPYRIGHT 2018 .********/
