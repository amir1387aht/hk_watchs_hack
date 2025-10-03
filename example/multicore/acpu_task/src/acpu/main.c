/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtconfig.h"
#include <board.h>
#include <string.h>
#include "acpu_ctrl.h"

/* ACPU main function */
void acpu_main(uint8_t task_name, void *param)
{
    switch (task_name)
    {
    case ACPU_TASK_0:
    {
        acpu_send_result("task_0", strlen("task_0") + 1);
        break;
    }
    case ACPU_TASK_1:
    {
        acpu_send_result("task_1", strlen("task_1") + 1);
        break;
    }
    default:
    {
        acpu_send_result("unknown task", strlen("unknown task") + 1);
        break;
    }
    }
}

