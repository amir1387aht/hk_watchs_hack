/**
  ******************************************************************************
  * @file   bf0_lib.h
  * @author Sifli software development team
  * @brief   This file contains all the functions prototypes for the Sifli library
  * @{
  ******************************************************************************
*/

#ifndef __BF0_LIB_H
#define __BF0_LIB_H
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int get_icon_transform_param(float x, float y, float icon_r, float *p_float_x, float *p_float_y, float *p_float_icon_r, float *p_float_pivot_r, float scr_width, float scr_height);

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

#endif /* __BF0_CP_HAL_H */

