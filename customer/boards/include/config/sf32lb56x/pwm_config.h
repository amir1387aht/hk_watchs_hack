/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PWM_CONFIG_H__
#define __PWM_CONFIG_H__

#include <rtconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PWM2_CORE CORE_ID_HCPU
#define PWM3_CORE CORE_ID_HCPU
#define PWM4_CORE CORE_ID_LCPU
#define PWM5_CORE CORE_ID_LCPU
#define PWM6_CORE CORE_ID_LCPU

#ifdef BSP_USING_PWM2
#define PWM2_CONFIG                             \
    {                                           \
       .tim_handle.Instance     = GPTIM1,       \
       .tim_handle.core         = PWM2_CORE,    \
       .name                    = "pwm2",       \
       .channel                 = 0             \
    }
#endif /* BSP_USING_PWM2 */

#ifdef BSP_USING_PWM3
#define PWM3_CONFIG                             \
    {                                           \
       .tim_handle.Instance     = GPTIM2,         \
       .tim_handle.core         = PWM3_CORE,    \
       .name                    = "pwm3",       \
       .channel                 = 0             \
    }
#endif /* BSP_USING_PWM3 */

#ifdef BSP_USING_PWM4
#define PWM4_CONFIG                             \
    {                                           \
       .tim_handle.Instance     = GPTIM3,         \
       .tim_handle.core         = PWM4_CORE,    \
       .name                    = "pwm4",       \
       .channel                 = 0             \
    }
#endif /* BSP_USING_PWM4 */

#ifdef BSP_USING_PWM5
#define PWM5_CONFIG                             \
    {                                           \
       .tim_handle.Instance     = GPTIM4,         \
       .tim_handle.core         = PWM5_CORE,    \
       .name                    = "pwm5",       \
       .channel                 = 0             \
    }
#endif /* BSP_USING_PWM5 */

#ifdef BSP_USING_PWM6
#define PWM6_CONFIG                             \
    {                                           \
       .tim_handle.Instance     = GPTIM5,         \
       .tim_handle.core         = PWM6_CORE,    \
       .name                    = "pwm6",       \
       .channel                 = 0             \
    }
#endif /* BSP_USING_PWM5 */

#ifdef BSP_USING_PWMA1
#define PWMA1_CONFIG                             \
        {                                           \
           .tim_handle.Instance     = (GPT_TypeDef *)ATIM1,         \
           .tim_handle.core         = ATIM1_CORE,    \
           .name                    = "pwma1",       \
           .channel                 = 0             \
        }
#endif /* BSP_USING_PWM_A1 */

#ifdef BSP_USING_PWMA2
#define PWMA2_CONFIG                             \
        {                                           \
           .tim_handle.Instance     = (GPT_TypeDef *)ATIM2,         \
           .tim_handle.core         = ATIM2_CORE,    \
           .name                    = "pwma2",       \
           .channel                 = 0             \
        }
#endif /* BSP_USING_PWM_A2 */

#ifdef BSP_USING_PWM_LPTIM1
#define PWM_LPTIM1_CONFIG                       \
    {                                           \
       .tim_handle.Instance     = hwp_lptim1,   \
       .name                    = "pwmlp1",       \
    }

#endif /* BSP_USING_PWM_LPTIM1 */

#ifdef BSP_USING_PWM_LPTIM2
#define PWM_LPTIM2_CONFIG                       \
    {                                           \
       .tim_handle.Instance     = hwp_lptim2,   \
       .name                    = "pwmlp2",       \
    }
#endif /* BSP_USING_PWM_LPTIM2 */

#ifdef BSP_USING_PWM_LPTIM3
#define PWM_LPTIM3_CONFIG                       \
    {                                           \
       .tim_handle.Instance     = hwp_lptim3,   \
       .name                    = "pwmlp3",       \
    }
#endif /* BSP_USING_PWM_LPTIM3 */
#ifdef BSP_PWM4_CC4_USING_DMA
#define PWM4_CC4_DMA_CONFIG                             \
{                                           \
   .dma_handle.Init.Priority = PWM4_CC4_DMA_IRQ_PRIO,           \
   .dma_handle.Instance = PWM4_CC4_DMA_INSTANCE,               \
   .dma_handle.Init.Request = PWM4_CC4_DMA_REQUEST,                 \
   .dma_irq = PWM4_CC4_DMA_IRQ,                     \
   .dma_handle_index = GPT_DMA_ID_CC4,                \
   .dma_handle.Init.PeriphDataAlignment    = PWM4_CC4_DMA_PDATAALIGN,         \
   .dma_handle.Init.MemDataAlignment   = PWM4_CC4_DMA_MDATAALIGN           \
}
#endif /* BSP_PWM4_CC4_USING_DMA */
#ifdef BSP_PWM5_CC1_USING_DMA
#define PWM5_CC1_DMA_CONFIG                             \
{                                           \
   .dma_handle.Init.Priority = PWM5_CC1_DMA_IRQ_PRIO,           \
   .dma_handle.Instance = PWM5_CC1_DMA_INSTANCE,               \
   .dma_handle.Init.Request = PWM5_CC1_DMA_REQUEST,                 \
   .dma_irq = PWM5_CC1_DMA_IRQ,                     \
   .dma_handle_index = GPT_DMA_ID_CC1,                \
   .dma_handle.Init.PeriphDataAlignment    = PWM5_CC1_DMA_PDATAALIGN,         \
   .dma_handle.Init.MemDataAlignment   = PWM5_CC1_DMA_MDATAALIGN           \
}
#endif /* BSP_PWM5_CC4_USING_DMA */

#ifdef __cplusplus
}
#endif

#endif /* __PWM_CONFIG_H__ */

