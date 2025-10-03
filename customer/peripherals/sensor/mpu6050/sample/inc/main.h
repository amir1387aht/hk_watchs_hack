/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MAIN_H
#define __MAIN_H


#include "stm32f4xx.h"
#include "discover_board.h"
#include <stdio.h>



#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void gyro_data_ready_cb(void);

#ifdef __cplusplus
}
#endif
#endif /* __MAIN_H */

