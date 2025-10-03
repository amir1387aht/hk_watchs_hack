/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ADC_CONFIG_H__
#define __ADC_CONFIG_H__

#include <rtconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSP_USING_ADC1
#ifndef ADC1_CONFIG
#define ADC1_CONFIG                                                 \
    {                                                               \
       .Instance                   = hwp_gpadc1,                          \
       .Init.atten3        = 0,             \
       .Init.adc_se            = 1,            \
       .Init.adc_force_on             = 0,           \
       .Init.dma_en          = 0,    \
       .Init.op_mode          = 0,           \
       .Init.en_slot      = 0,                       \
       .Init.data_samp_delay        = 2,          \
       .Init.conv_width        = 75,          \
       .Init.sample_width        = 71,          \
    }
#endif /* ADC1_CONFIG */

#ifdef BSP_ADC1_USING_DMA
#ifndef ADC1_DMA_CONFIG
#define ADC1_DMA_CONFIG                               \
    {                                                 \
        .dma_irq_prio = GPADC_IRQ_PRIO,               \
        .Instance = GPADC_DMA_INSTANCE,               \
        .dma_irq = GPADC_DMA_IRQ,                     \
        .request = GPADC_DMA_REQUEST,                 \
    }

#endif  /* ADC_DMA_CONFIG */
#endif  /* BSP_ADC_USING_DMA */

#endif /* BSP_USING_ADC1 */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_CONFIG_H__ */

