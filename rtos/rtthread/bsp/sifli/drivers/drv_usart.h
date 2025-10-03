/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>
#include <drv_common.h>

int rt_hw_usart_init(void);

#define DMA_INSTANCE_TYPE              DMA_Channel_TypeDef

#define UART_INSTANCE_CLEAR_FUNCTION    __HAL_UART_CLEAR_FLAG

/* sifli uart config class */
struct sifli_uart_config
{
    const char         *name;
    USART_TypeDef      *Instance;
    IRQn_Type           irq_type;
    struct dma_config *dma_rx;
    struct dma_config *dma_tx;
};

/* sifli uart dirver class */
struct sifli_uart
{
    UART_HandleTypeDef handle;
    struct sifli_uart_config *config;

#ifdef RT_SERIAL_USING_DMA
    struct
    {
        DMA_HandleTypeDef handle;
        rt_size_t last_index;
    } dma_rx;
    struct
    {
        DMA_HandleTypeDef handle;
        rt_size_t last_index;
    } dma_tx;
#endif
    rt_uint8_t uart_rx_dma_flag;
    rt_uint8_t uart_tx_dma_flag;
    struct rt_serial_device serial;
};

#endif  /* __DRV_USART_H__ */

