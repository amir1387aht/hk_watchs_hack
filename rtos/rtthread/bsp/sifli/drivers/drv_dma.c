/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <drv_log.h>
#include <drv_common.h>

#ifdef DMA_SUPPORT_DYN_CHANNEL_ALLOC
void DMAC1_CH1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH1_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC1_CH2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH2_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC1_CH3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH3_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC1_CH4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH4_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC1_CH5_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH5_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC1_CH6_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH6_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC1_CH7_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH7_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC1_CH8_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC1_CH8_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH1_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH2_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH3_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH4_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH5_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH5_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH6_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH6_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH7_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH7_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC2_CH8_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC2_CH8_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

#if defined(DMA3)
/* SF32LB58X LCPU still use ROM implementation */
void DMAC3_CH1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH1_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC3_CH2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH2_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC3_CH3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH3_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC3_CH4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH4_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC3_CH5_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH5_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC3_CH6_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH6_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC3_CH7_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH7_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}

void DMAC3_CH8_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMAC3_CH8_IRQHandler();

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* DMA3 */
#endif /* DMA_SUPPORT_DYN_CHANNEL_ALLOC */

/// @} file

