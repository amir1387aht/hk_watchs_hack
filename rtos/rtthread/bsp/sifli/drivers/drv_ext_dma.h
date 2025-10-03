/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_EXT_DMA_H_
#define __DRV_EXT_DMA_H_

#include <rtthread.h>
#include <board.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bsp_driver Driver IO
  * @{
  */

/** @defgroup drv_ext_dma EXT_DMA
  * @brief EXT_DMA BSP driver
  * @{
  */

typedef void (* pCallback)(void);

typedef enum
{
    EXT_DMA_XFER_CPLT_CB_ID          = 0x00U,    /*!< Full transfer     */
    EXT_DMA_XFER_ERROR_CB_ID         = 0x01U,    /*!< Error             */
} EXT_DMA_CallbackIDTypeDef;

typedef struct
{
    bool cmpr_en;
    uint8_t cmpr_rate;
    uint16_t col_num;
    uint16_t row_num;
    uint32_t src_format;
} EXT_DMA_CmprTypeDef;

/**
 * @brief Configure ext_dma with source, dest address auto increase mode.
 * @param[in] src_inc: Source address auto increase
 * @param[in] dst_inc: Dest address auto increase
 * @return configure result, 0 success
 */
rt_err_t EXT_DMA_Config(uint8_t src_inc, uint8_t dst_inc);
/**
 * @brief Configure ext_dma with source, dest address auto increase mode.
 * @param[in] src_inc Source address auto increase
 * @param[in] dst_inc Dest address auto increase
 * @param[in] cmpr compression configuration
 * @return configure result, 0 success
 */
rt_err_t EXT_DMA_ConfigCmpr(uint8_t src_inc, uint8_t dst_inc, const EXT_DMA_CmprTypeDef *cmpr);

rt_err_t EXT_DMA_START_ASYNC(uint32_t src, uint32_t dst, uint32_t len);
void EXT_DMA_Register_Callback(EXT_DMA_CallbackIDTypeDef cid, pCallback cb);

/**
 * @brief Get ext-dma error code
 * @return error code
 */
uint32_t EXT_DMA_GetError(void);

rt_err_t EXT_DMA_TRANS_SYNC(uint32_t src, uint32_t dst, uint32_t len, uint32_t timeout);

void EXT_DMA_Wait_ASYNC_Done(void);

/// @} drv_dma
/// @} bsp_driver

#ifdef __cplusplus
}
#endif

#endif /*__DRV_DMA_H_ */

/// @} file

