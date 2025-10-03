/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "board.h"

int main(void)
{
    while (1)
    {
#ifndef SF32LB52X        
        if (HAL_PMU_LXT_DISABLED())
        {
            HAL_RC_CAL_update_reference_cycle_on_48M(LXT_LP_CYCLE);
            rt_thread_mdelay(15 * 1000);
        }
        else
        {
            rt_thread_mdelay(3600000);
            break;
        }
#else
        /* disable all timer to make LCPU sleep */
        break;
#endif /* SF32LB52X */
    }
    return RT_EOK;
}

