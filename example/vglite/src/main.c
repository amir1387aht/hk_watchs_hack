/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

#ifdef BSP_USING_TOUCHD
    #include "drv_touch.h"
#endif
#include "mem_section.h"

int main(void)
{

    while (1)
    {
        rt_thread_mdelay(5000);
        rt_kprintf("__main loop__\r\n");
    }

    return RT_EOK;
}

