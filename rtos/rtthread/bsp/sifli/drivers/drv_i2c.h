/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <drv_common.h>

typedef struct bf0_i2c_config
{
    const char *device_name;
    I2C_TypeDef *Instance;
    IRQn_Type irq_type;
    uint8_t core;
    struct dma_config *dma_rx;
    struct dma_config *dma_tx;
} bf0_i2c_config_t;

typedef struct bf0_i2c
{
    struct rt_i2c_bus_device bus;
    I2C_HandleTypeDef handle;
    bf0_i2c_config_t *bf0_i2c_cfg;
    struct
    {
        DMA_HandleTypeDef dma_rx;
        DMA_HandleTypeDef dma_tx;
    } dma;
    rt_uint8_t i2c_dma_flag;
} bf0_i2c_t;

enum
{
#ifdef BSP_USING_I2C1
    I2C1_INDEX,
#endif
#ifdef BSP_USING_I2C2
    I2C2_INDEX,
#endif
#ifdef BSP_USING_I2C3
    I2C3_INDEX,
#endif
#ifdef BSP_USING_I2C4
    I2C4_INDEX,
#endif
#ifdef BSP_USING_I2C5
    I2C5_INDEX,
#endif
#ifdef BSP_USING_I2C6
    I2C6_INDEX,
#endif
#ifdef BSP_USING_I2C7
    I2C7_INDEX,
#endif
    I2C_MAX,
};

int rt_hw_i2c_init(void);

#endif

