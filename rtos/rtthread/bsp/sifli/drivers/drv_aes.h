/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_AES_H_
#define __DRV_AES_H_

#include <rtthread.h>
#include <board.h>
#include "bf0_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bsp_driver Driver IO
  * @{
  */

/** @defgroup drv_aes AES
  * @brief AES BSP driver
  * @{
  */

typedef struct
{
    uint32_t *key;
    int key_size; //16:AES_KEY_128,  24: AES_KEY_192,  32:AES_KEY_LEN_256.
    uint32_t *iv;
    uint32_t mode; //0:ECB  1:CTR  2:CBC
} AES_KeyTypeDef;

typedef struct
{
    uint8_t *in_data;
    uint8_t *out_data;
    uint32_t size;
} AES_IOTypeDef;

/**
 * @brief encode/decode data in interrupt mode.
 * @param cfg - configuration of enc/dec key , inital vector, and mode.
 * @param data - input&output data
 * @param cb - finish callback
 * @return RT_EOK if successful
 */
rt_err_t drv_aes_enc_async(AES_KeyTypeDef *cfg, AES_IOTypeDef *data, pAESCallback cb);
rt_err_t drv_aes_dec_async(AES_KeyTypeDef *cfg, AES_IOTypeDef *data, pAESCallback cb);

/**
 * @brief encode/decode data in CPU polling mode.
 * @param cfg - configuration of enc/dec key , inital vector, and mode.
 * @param data - input&output data
 * @param cb - finish callback
 * @return RT_EOK if successful
 */
rt_err_t drv_aes_enc_sync(AES_KeyTypeDef *cfg, AES_IOTypeDef *data);
rt_err_t drv_aes_dec_sync(AES_KeyTypeDef *cfg, AES_IOTypeDef *data);

/**
 * @brief copy data like DMA in interrupt mode.
 * @param data - input&output data
 * @param cb - finish callback
 * @return RT_EOK if successful
 */
rt_err_t drv_aes_copy_async(AES_IOTypeDef *data, pAESCallback cb);

/// @} drv_aes
/// @} bsp_driver

#ifdef __cplusplus
}
#endif

#endif /*__DRV_AES_H_ */

/// @} file

