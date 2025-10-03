/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef EZIPA_DEC_H
#define EZIPA_DEC_H
#include <rtthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <bf0_hal.h>

#ifdef EZIPA_CUSTOM_INCLUDE_FILE
    #include EZIPA_CUSTOM_INCLUDE_FILE
#endif /* EZIPA_CUSTOM_INCLUDE_FILE */

/**
 ****************************************************************************************
* @addtogroup ezipa_dec eZIP-A Decoder
* @ingroup middleware
* @brief EZIPA Decoder interface
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Large buffer allocate */
#ifndef EZIPA_LARGE_BUF_MALLOC
#define EZIPA_LARGE_BUF_MALLOC(size)  rt_malloc((size))
#elif !defined(EZIPA_LARGE_BUF_FREE)
#error "EZIPA_LARGE_BUF_FREE must be defined at the same time"
#endif /* EZIPA_IMG_MALLOC */

/** Large buffer free */
#ifndef EZIPA_LARGE_BUF_FREE
#define EZIPA_LARGE_BUF_FREE(ptr)     rt_free((ptr))
#elif !defined(EZIPA_LARGE_BUF_MALLOC)
#error "EZIPA_LARGE_BUF_MALLOC must be defined at the same time"
#endif /* EZIPA_LARGE_BUF_FREE */

typedef enum
{
    EZIPA_RGB565,
    EZIPA_RGB888,
} ezipa_color_fmt_t;

typedef struct
{
    uint8_t *ezipa_data;
    uint8_t *output_buf;
    /* backup region buffer for disp_op=2 */
    uint8_t *region_area_bak_buf;
    uint8_t output_color_fmt;
    bool alpha_enabled;
    uint8_t epic_color_fmt;
    uint8_t pixel_size;
    uint16_t frame_delay_ms;
    uint32_t play_idx;
    EZIP_HandleTypeDef *ezip_handle;
    EZIP_EZIPAHeaderTypeDef header;
    EZIP_EZIPAFrameInfoTypeDef curr_frame;
    EZIP_EZIPAFrameInfoTypeDef next_frame;
    bool valid_curr_frame;
    struct rt_semaphore sem;
    /* following fields are used by file  */
    int fd;
    const char *filename;
    uint32_t file_size;
    uint32_t ezipa_hdr_size;
    /* including one frame and next frame header */
    uint32_t max_frame_size;
    uint32_t *org_frame_offset_tbl;
    uint32_t *fake_frame_offset_tbl;
    uint32_t frame_num;
} ezipa_obj_t;

typedef struct
{
    /** canvas buffer */
    uint8_t *buf;
    /** canvas buffer color format, EPIC color format, e.g. #EPIC_COLOR_ARGB8565 */
    uint8_t color_fmt;
    /** canvas width */
    uint16_t width;
    /** canvas height */
    uint16_t height;

    /** x offset where ezipa frame is drawn on canvas */
    int16_t x_offset;
    /** y offset where ezipa frame is drawn on canvas */
    int16_t y_offset;

    int32_t mask_x_offset;
    int32_t mask_y_offset;
    int32_t mask_width;
    int32_t mask_height;
} ezipa_canvas_t;

/**
 * @brief  Allocate ezipa drawing object
 *
 *  Read the ezipa data, allocate the resource and prepare for the drawing
 *
 * @param[in]  data ezipa const data or file name
 * @param[in]  output_color_fmt output buffer color format
 *
 * @retval ezipa object instance
 */
ezipa_obj_t *ezipa_open(const void *data, ezipa_color_fmt_t output_color_fmt);

/**
 * @brief  Free ezipa drawing object
 *
 * @param[in]  obj ezipa object instance
 *
 * @retval 0: no error, < 0: error code
 */
int32_t ezipa_close(ezipa_obj_t *obj);

/**
 * @brief  Draw one frame on the canvas
 *
 * @param[in]  obj ezipa object instance
 * @param[in]  canvas canvas
 * @param[in]  next true: draw next frame on the canvas, false: draw current frame on the canvas
 *
 * @retval 0: no error, < 0: error code
 */
int32_t ezipa_draw(ezipa_obj_t *obj, ezipa_canvas_t *canvas, bool next);

/// @}  ezipa_dec

#ifdef __cplusplus
}
#endif

/// @} file
#endif

