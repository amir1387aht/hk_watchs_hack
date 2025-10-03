/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CMT8530S_SMT_TR_H__
#define __CMT8530S_SMT_TR_H__



#include <rtthread.h>
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

int cmt8530s_init();
int cmt8530s_open();
int cmt8530s_set_freq(uint32_t freq);
int cmt8530s_close();


#ifdef __cplusplus
}
#endif
#endif /* __CMT8530S_SMT_TR_H__ */
