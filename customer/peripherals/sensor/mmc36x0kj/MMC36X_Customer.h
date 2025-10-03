/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MMC36X_HDR_FILE__
#define __MMC36X_HDR_FILE__


#include "board.h"
/**
 * @brief Delay function.
 */

#ifdef __cplusplus
extern "C" {
#endif

void Delay_Ms(int cnt);

/**
 * @brief Delay function.
 */
void Delay_Us(int cnt);

/**
 * @brief I2C BUS initial.
 */
int MMC36X_I2C_Init();
/**
 * @brief I2C write register.
 */
int I2C_Write_Reg(unsigned char i2c_add, unsigned char reg_add, unsigned char cmd);

/**
 * @brief I2C read register.
 */
int I2C_Read_Reg(unsigned char i2c_add, unsigned char reg_add, unsigned char *data);

/**
 * @brief I2C multi read.
 */
int I2C_MultiRead_Reg(unsigned char i2c_add, unsigned char reg_add, int num, unsigned char *data);

uint32_t MMC36X0KJ_get_bus();
int MMC36X0KJ_self_check(void);

#ifdef __cplusplus
}
#endif

#endif //__MMC36X_HDR_FILE__
