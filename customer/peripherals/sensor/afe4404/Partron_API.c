/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "board.h"
#include "afe4404_hw.h"

/******************************************************************************/
void PPSI26X_enableRead(void);
void PPSI26X_enableWrite(void);
int PPSI26X_readRegWithoutReadEnable(int reg);
int PPSI26X_readRegWithReadEnable(int reg);
void PPSI26X_writeRegWithoutWriteEnable(int reg, int registerValue);
void PPSI26X_writeRegWithWriteEnable(int reg, int registerValue);
void check_inside(int value1, int value2, int value3);

/******************************************************************************/

/**
 * @brief Writes to a HW register by I2C communication
 * @param regAddress: Register address to write to
 * @param wdata : Data to send to the address
 * @return: none
 */
static void I2C_writeReg(int regAddress, int wdata)
{
    /* I2C Write */
    //I2C_writeReg_Internal(regAddress,wdata);
    PPS960_writeReg((uint8_t)(regAddress & 0xff), wdata);
}

/**
 * @brief Read data from a register
 * @param regAddress: Register to read from
 * @return: Return the register data in uint32
 */
static int I2C_readReg(int regAddress)
{
    /* I2C Read */
    return PPS960_readReg((uint8_t)(regAddress & 0xff));
}

/**
 * @brief: Enables the AFE to read to a register
 * @param: None
 * @return: None
 */
void PPSI26X_enableRead(void)
{
    I2C_writeReg(0x00, 0x00000021);
}

/**
 * @brief: Enables the AFE to write to a register
 * @param: None
 * @return: None
 */
void PPSI26X_enableWrite(void)
{
    I2C_writeReg(0x00, 0x00000020);
}

int PPSI26X_readRegWithoutReadEnable(int reg)
{
    return (I2C_readReg(reg));
}

int PPSI26X_readRegWithReadEnable(int reg)
{
    PPSI26X_enableRead();
    return (I2C_readReg(reg));
}

void PPSI26X_writeRegWithoutWriteEnable(int reg, int registerValue)
{
    I2C_writeReg(reg, registerValue);
}

void PPSI26X_writeRegWithWriteEnable(int reg, int registerValue)
{
    PPSI26X_enableWrite();
    I2C_writeReg(reg, registerValue);
}

