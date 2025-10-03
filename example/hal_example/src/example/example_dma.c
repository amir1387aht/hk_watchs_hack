/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdlib.h>
#include "bf0_hal.h"
#include "utest.h"
#include "tc_utils.h"

/*
    This example demo:
        1. Copy 32KB data from flash to PSRAM with DMAC1 channel 1 in asynchronize mode.
        2. Content of data on flash should match with data on PSRAM
*/

static DMA_HandleTypeDef testdma;

static void dma_done_cb()
{
    LOG_I("DMA Done.");
}

static void dma_err_cb()
{
    LOG_I("DMA Error!");
}

#ifndef DMA_SUPPORT_DYN_CHANNEL_ALLOC
void DMAC1_CH1_IRQHandler(void)
{
    /* enter interrupt */
    ENTER_INTERRUPT();

    HAL_DMA_IRQHandler(&testdma);

    /* leave interrupt */
    LEAVE_INTERRUPT();
}
#endif /* !DMA_SUPPORT_DYN_CHANNEL_ALLOC */

static void testcase(int argc, char **argv)
{

    uint32_t src, dst, len_in_words;
    HAL_StatusTypeDef res;

#ifdef SOC_SF32LB55X
    src = 0x10000000;
    dst = 0x600a0000;
#elif defined(SOC_SF32LB56X)
    src = 0x10000000;
    dst = 0x60200000;
#ifdef BSP_USING_BOARD_EC_LB567XXX
    src = 0x10800000;
    dst = 0x60a00000;
#endif  //BSP_USING_BOARD_EC_LB567XXX
#elif defined(SOC_SF32LB52X)
    src = 0x12000000;
    dst = 0x60000000;
#else
    src = 0x10200000;
    dst = 0x62200000;
#endif

    len_in_words = 8 * 1024; //8K words == 32K bytes

    testdma.Instance = DMA1_Channel1;
    testdma.Init.Request = 0; //DMA_REQUEST_MEM2MEM;
    testdma.Init.Direction = DMA_MEMORY_TO_MEMORY;
    testdma.Init.PeriphInc = DMA_PINC_ENABLE;
    testdma.Init.MemInc = DMA_MINC_ENABLE;
    testdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    testdma.Init.MemDataAlignment   = DMA_MDATAALIGN_WORD;
    testdma.Init.Mode               = DMA_NORMAL;
    testdma.Init.Priority           = DMA_PRIORITY_HIGH;
    testdma.Init.PeriphInc = DMA_PINC_ENABLE; //src
    testdma.Init.MemInc = DMA_MINC_ENABLE;    //dst

    HAL_DMA_Init(&testdma);

    LOG_I("dma  src:%x(4) dst:%x(4) len: %d bytes", src, dst, len_in_words * 4);

#ifndef DMA_SUPPORT_DYN_CHANNEL_ALLOC
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(DMAC1_CH1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMAC1_CH1_IRQn);
#endif /* !DMA_SUPPORT_DYN_CHANNEL_ALLOC */

    HAL_DMA_RegisterCallback(&testdma, HAL_DMA_XFER_CPLT_CB_ID, dma_done_cb);
    HAL_DMA_RegisterCallback(&testdma, HAL_DMA_XFER_ERROR_CB_ID, dma_err_cb);

    HAL_DMA_Start_IT(&testdma, (uint32_t)src, (uint32_t)dst, len_in_words);

    rt_thread_mdelay(1000);

    if (memcmp((void *)src, (void *)dst, len_in_words * 4) == 0)       // Destination data should match with source input
        LOG_I("DMA src&dst matched.");
    else
        LOG_I("DMA Error.");

}

UTEST_TC_EXPORT(testcase, "example_dma", NULL, NULL, 10);

