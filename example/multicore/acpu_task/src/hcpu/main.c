/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

#define DBG_TAG "main"
#define DBG_LEVEL  DBG_INFO

#include "log.h"
#include "acpu_ctrl.h"

int main(void)
{
    while (1)
    {
        rt_thread_mdelay(10000);
    }

    return RT_EOK;
}

static int run_acpu(int argc, char *argv[])
{
    char *s;

    if (argc < 2)
    {
        rt_kprintf("wrong argument\n");
        return -1;
    }

    /* config ACPU to run specified task */
    s = acpu_run_task(atoi(argv[1]) + ACPU_TASK_0, NULL, 0, NULL);

    LOG_I(s);

    return 0;
}
MSH_CMD_EXPORT(run_acpu, "run acpu")

