/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>
#include <drv_common.h>

//rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin);
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name);

struct sifli_hw_spi_cs
{
    //GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
};

struct sifli_spi_config
{
    SPI_TypeDef *Instance;
    char *bus_name;
    IRQn_Type irq_type;
    uint8_t core;
    struct dma_config *dma_rx, *dma_tx;
};

struct sifli_spi_device
{
    rt_uint32_t pin;
    char *bus_name;
    char *device_name;
};

#define SPI_USING_RX_DMA_FLAG   (1<<0)
#define SPI_USING_TX_DMA_FLAG   (1<<1)

/* sifli spi dirver class */
struct sifli_spi
{
    SPI_HandleTypeDef handle;
    struct sifli_spi_config *config;
    struct rt_spi_configuration *cfg;

    struct
    {
        DMA_HandleTypeDef handle_rx;
        DMA_HandleTypeDef handle_tx;
    } dma;

    rt_uint8_t spi_dma_flag;
    struct rt_spi_bus spi_bus;
    rt_sem_t spi_sema;
};

#endif /*__DRV_SPI_H_ */

