/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MMC36X_Customer.h"
#include "board.h"
#include <stdlib.h>
#include "i2c.h"

#define DBG_TAG "drv.mmc36x"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

// move it to menuconfig if needed
//#define MMC36X0KJ_I2C_BUS         "i2c2"

static struct rt_i2c_bus_device *mmc_i2cbus = NULL;

void Delay_Ms(int cnt)
{
    /*  cnt is the time to wait */

    /*
    .
    . Need to be implemented by user.
    .
    */

    rt_thread_delay(cnt);

    return;
}

void Delay_Us(int cnt)
{
    /*  cnt is the time to wait */

    /*
    .
    . Need to be implemented by user.
    .
    */

    return;
}

int MMC36X_I2C_Init()
{
    /* get i2c bus device */
    mmc_i2cbus = rt_i2c_bus_device_find(MMC36X0KJ_I2C_BUS);
    if (mmc_i2cbus)
    {
        rt_i2c_open(mmc_i2cbus, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
        //rt_i2c_open(mmc_i2cbus, RT_DEVICE_FLAG_RDWR);
        LOG_I("Find i2c bus device %s\n", MMC36X0KJ_I2C_BUS);
    }
    else
    {
        LOG_E("Can not found i2c bus %s, init fail\n", MMC36X0KJ_I2C_BUS);
        return -1;
    }

    return 1;
}

int I2C_Write_Reg(unsigned char i2c_add, unsigned char reg_add, unsigned char cmd)
{
    /* i2c_add is the 7-bit i2c address of the sensor
     * reg_add is the register address to wtite
     * cmd is the value that need to be written to the register
     * I2C operating successfully, return 1, otherwise return 0;
     */

    /*
    .
    . Need to be implemented by user.
    .
    */
    struct rt_i2c_msg msgs;
    uint8_t value[2];
    uint32_t res;

    if (mmc_i2cbus)
    {
        value[0] = reg_add;
        value[1] = cmd;

        msgs.addr  = i2c_add;    /* Slave address */
        msgs.flags = RT_I2C_WR;        /* Write flag */
        msgs.buf   = value;             /* Slave register address */
        msgs.len   = 2;                /* Number of bytes sent */

        //LOG_I("I2c write dev 0x%x, addr 0x%x, val 0x%x\n", i2c_add, reg_add, cmd);
        res = rt_i2c_transfer(mmc_i2cbus, &msgs, 1);
        if (res != 1)
        {
            LOG_I("I2C_Write_Reg FAIL %d\n", res);
            return 0;
        }

    }

    return 1;
}
int I2C_Read_Reg(unsigned char i2c_add, unsigned char reg_add, unsigned char *data)
{
    /* i2c_add is the 7-bit i2c address of the sensor
     * reg_add is the register address to read
     * data is the first address of the buffer that need to store the register value
     * I2C operating successfully, return 1, otherwise return 0;
     */

    /*
    .
    . Need to be implemented by user.
    .
    */
    struct rt_i2c_msg msgs[2];
    uint32_t res;
    uint8_t value;

    if (mmc_i2cbus)
    {
        msgs[0].addr  = i2c_add;    /* Slave address */
        msgs[0].flags = RT_I2C_WR;        /* Write flag */
        msgs[0].buf   = &reg_add;             /* Slave register address */
        msgs[0].len   = 1;                /* Number of bytes sent */

        msgs[1].addr  = i2c_add;    /* Slave address */
        msgs[1].flags = RT_I2C_RD;        /* Read flag */
        msgs[1].buf   = data;              /* Read data pointer */
        msgs[1].len   = 1;              /* Number of bytes read */

        //LOG_I("I2c read dev 0x%x, addr 0x%x\n", i2c_add, reg_add);
        res = rt_i2c_transfer(mmc_i2cbus, msgs, 2);
        if (res != 2)
        {
            LOG_I("I2C_Read_Reg fail %d\n", res);
            return 0;
        }
    }

    return 1;
}
int I2C_MultiRead_Reg(unsigned char i2c_add, unsigned char reg_add, int num, unsigned char *data)
{
    /* i2c_add is the 7-bit i2c address of the sensor
     * reg_add is the first register address to read
     * num is the number of the register to read
     * data is the first address of the buffer that need to store the register value
     * I2C operating successfully, return 1, otherwise return 0;
     */

    /*
    .
    . Need to be implemented by user.
    .
    */
#if 0
    struct rt_i2c_msg msgs[2];
    uint32_t res;

    if (mmc_i2cbus)
    {
        msgs[0].addr  = i2c_add;    /* Slave address */
        msgs[0].flags = RT_I2C_WR;        /* Write flag */
        msgs[0].buf   = &reg_add;             /* Slave register address */
        msgs[0].len   = 1;                /* Number of bytes sent */

        msgs[1].addr  = i2c_add;    /* Slave address */
        msgs[1].flags = RT_I2C_RD;        /* Read flag */
        msgs[1].buf   = data;              /* Read data pointer */
        msgs[1].len   = num;              /* Number of bytes read */

        LOG_I("I2c mread dev 0x%x, addr 0x%x size %d\n", i2c_add, reg_add, num);
        res = rt_i2c_transfer(mmc_i2cbus, msgs, 2);
        if (res != 2)
        {
            LOG_I("I2C_Read_Reg fail %d\n", res);
            return 0;
        }
    }

#else
    uint32_t res;
    if (mmc_i2cbus)
    {
        LOG_I("I2c mread dev 0x%x, addr 0x%x size %d\n", i2c_add, reg_add, num);
        res = rt_i2c_mem_read(mmc_i2cbus, i2c_add, reg_add, 8, data, num);
        if (res <= 0)
        {
            LOG_I("I2C_MultiRead_Reg fail %d\n", res);
            return 0;
        }
    }
#endif
    return 1;
}

uint32_t MMC36X0KJ_get_bus()
{
    return (uint32_t)mmc_i2cbus;
}

int MMC36X0KJ_self_check(void)
{
    int ret = 0;
    extern int MMC36X0KJ_Check_OTP(void);
    extern int MMC36X0KJ_CheckID(void);
    /* Check OTP Read status */
    ret = MMC36X0KJ_Check_OTP();
    if (ret < 0)
        return ret;

    /* Check product ID */
    ret = MMC36X0KJ_CheckID();
    if (ret < 0)
        return ret;

    return 0;
}

#define DRV_MMC36X_TEST
#ifdef DRV_MMC36X_TEST

#include <string.h>
#include "MMC36X0KJ.h"

extern void mmc_sample_start(void);
extern int mmc_get_direction(void);
extern void mmc_comp_log(int en);
extern int mmc_comp_is_accu(void);

int mmc36x_test(int argc, char *argv[])
{
    int ret;
    if (argc < 2)
    {
        LOG_I("Invalid parameter\n");
        return 0;
    }

    if (strcmp(argv[1], "-open") == 0)
    {
        ret = MMC36X0KJ_Initialization();
        if (ret > 0)
        {
            LOG_I("MMC36X0KJ open success\n");
            MMC36X0KJ_Enable();
        }
        else
        {
            LOG_I("MMC36X0KJ open fail ret\n", ret);
        }
    }
    else if (strcmp(argv[1], "-temp") == 0)
    {
        float temp = 0.0;
        MMC36X0KJ_GetTemperature(&temp);
        LOG_I("Current temperature = %f degree\n", temp);
    }
    else if (strcmp(argv[1], "-mag") == 0)
    {
        float mag[3];
        MMC36X0KJ_GetData(mag);
        LOG_I("MagX = %04f, magY = %04f, magZ = %04f\n", mag[0], mag[1], mag[2]);
    }
    else if (strcmp(argv[1], "-start") == 0)    // start compass sample loop
    {
        ret = MMC36X0KJ_Initialization();
        if (ret > 0)
        {
            MMC36X0KJ_Enable();
            mmc_sample_start();
        }
        else
            LOG_I("Mag initial fail\n");
    }
    else if (strcmp(argv[1], "-dir") == 0)
    {
        int dir = mmc_get_direction();
        switch (dir)
        {
        case 0:
            LOG_I("North\n");
            break;
        case 1:
            LOG_I("North East\n");
            break;
        case 2:
            LOG_I("East\n");
            break;
        case 3:
            LOG_I("South East\n");
            break;
        case 4:
            LOG_I("South\n");
            break;
        case 5:
            LOG_I("South West\n");
            break;
        case 6:
            LOG_I("West\n");
            break;
        case 7:
            LOG_I("North West\n");
            break;
        default:
            LOG_I("Not detected\n");
            break;
        }
    }
    else if (strcmp(argv[1], "-log") == 0)
    {
        int logen = atoi(argv[2]);
        mmc_comp_log(logen);
    }
    else if (strcmp(argv[1], "-ok") == 0)
    {
        int done = mmc_comp_is_accu();
        if (done >= 1)
            LOG_I("data ready!\n");
        else
            LOG_I("data not ready\n");
    }
    else
    {
        LOG_I("invalid parameter\n");
    }

    return 0;

}

FINSH_FUNCTION_EXPORT_ALIAS(mmc36x_test, __cmd_mmc, Test hw mmc);
#endif //DRV_MMC36X_TEST

