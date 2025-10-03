/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "board.h"
#include "step_wraper.h"
#include "lsm6dsl.h"
#include "counter.h"

#ifdef STEP_USING_VIRTUAL

rt_thread_t vsteo_thread = NULL;

static void virt_step_sensor_task(void *params)  //20ms
{
    int32_t ret;
    int debug_cnt = 0;
    int accx = 0, accy = 0, accz = 0;
    SportDataType data = {0};

    while (1)
    {
        ret = lsm6dsl_accel_read(&accx, &accy, &accz);
        if (ret != 0)
            rt_kprintf("get accel data fail\n");
        Sport_Calculator(accx, accy, accz); // suppose parameter mg based

        debug_cnt++;    // remove it if close debug
        if ((debug_cnt & 0x7f) == 0)    // output each 128 * 20 ms
        {
            Read_SportData(&data);
            rt_kprintf("Step count %d\n", data.steps);
            //rt_kprintf("Input 0x%x, 0x%x, 0x%x\n", accx, accy, accz);
        }

        rt_thread_delay(20);    // continue check with 50hz
    }
}

int virt_step_init(void)
{
    int res;

    res = lsm6dsl_init();
    if (res != 0)
        return res;

    Sport_Init();
    Set_Parameter(175, 80);
    vsteo_thread = rt_thread_create("virt_step", virt_step_sensor_task, NULL, 1024, RT_THREAD_PRIORITY_HIGH, RT_THREAD_TICK_DEFAULT);
    if (vsteo_thread != NULL)
    {
        rt_thread_startup(vsteo_thread);
        rt_kprintf("step thread started\n");
    }
    else
    {
        rt_kprintf("Create step thread fail\n");
        return 1;
    }

    return 0;
}

int vstep_get_step()
{
    SportDataType data = {0};

    Read_SportData(&data);

    return (int)data.steps;
}

int vstep_get_distance()
{
    SportDataType data = {0};

    Read_SportData(&data);

    return (int)data.dist;
}

#define DRV_VSTEO_TEST
#ifdef DRV_VSTEO_TEST

int vstep_test(int argc, char *argv[])
{
    if (argc < 2)
    {
        rt_kprintf("Invalid parameter\n");
        return 0;
    }

    if (strcmp(argv[1], "-open") == 0) // pedometer lib-c
    {
        int res = virt_step_init();
        if (res == 0)
            rt_kprintf("Step sensor start\n");
        else
            rt_kprintf("Step sensor start fail\n");
    }
    else if (strcmp(argv[1], "-s") == 0) // pedometer lib-c
    {
        rt_kprintf("Step %d\n", vstep_get_step());
    }
    else if (strcmp(argv[1], "-d") == 0) // pedometer lib-c
    {
        rt_kprintf("Distance %d\n", vstep_get_distance());
    }
    else
    {
        rt_kprintf("Invalid parameter\n");
    }

    return 0;
}

FINSH_FUNCTION_EXPORT_ALIAS(vstep_test, __cmd_step, Test hw step);
#endif //DRV_VSTEO_TEST

#endif /*STEP_USING_VIRTUAL*/

