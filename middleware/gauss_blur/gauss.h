/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __GAUSS_H__
#define __GAUSS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*Gauss interface*/
typedef struct
{
    uint8_t *data;            /**< data, color format is specified by #color_mode,
                                   point to the pixel data corresponding to pixel(#x_offset,#y_offset) */
    uint32_t color_mode;      /**< color mode, refer to #EPIC_COLOR_RGB565 */
    uint16_t width;           /**< layer width to be processed */
    uint16_t height;          /**< layer heigth to be processed  */
} BlurDataType;

typedef void (*gauss_cbk)(void);

/**
* @brief  Initialize Gauss module
* @param  in Input data, image to be blured.
* @param  out Output data, image blured
* @param  radius blur kernal size, typical 60.
* @retval  RT_EOK if success, otherwise failed.
*/
void *gauss_init(BlurDataType *in, BlurDataType *out, uint16_t radius);

/**
 * @brief  De-Initialize Gauss module
 * @retval  RT_EOK if success, otherwise failed.
*/
int gauss_deinit(void *gauss);

/**
 * @brief  Trigger Gauss calculation
 * @param   cbk, callback function called when finish.
 * @retval  RT_EOK if success, otherwise failed.
*/
int gauss_start_IT(void *gauss, gauss_cbk cbk);

#ifdef __cplusplus
}
#endif

#endif /* __GAUSS_H__ */

