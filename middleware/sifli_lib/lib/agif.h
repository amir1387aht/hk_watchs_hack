/*
 * SPDX-FileCopyrightText: 2019-2024 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AGIF_H_
#define _AGIF_H_


#include <stdint.h>
#include <sys/types.h>

#ifndef DFU_OTA_MANAGER
    #include "lvgl.h"
    #if LVGL_V9
        #include "lv_image_private.h"
    #endif
    #define ANIM_LV_TASK
    // #include "app_mem.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GIF_REFR_PEROID                     _lv_refr_get_disp_refreshing()->refr_timer->period
#define GIF_PALETTE_TO_PIXEL
#define GIF_ZOOM_NEED

typedef enum
{
    AGIF_FS_DATA_NORMAL_MODE,
    AGIF_FS_DATA_CONTINUE_MODE,
} agif_fs_mode_t;

typedef struct
{
    uint16_t    width, height;          /**< gif width and height                           */
    uint16_t    fx, fy, fw, fh;         /**< current gif pic area                           */
    uint8_t    *data;                   /**< gif src                                        */
    uint32_t    data_size;              /**< gif src data size                              */
    void       *gif_handle;             /**< gif handle, store gif_t                        */
    uint32_t    delay;                  /**< waiting time before decode gif                 */
    uint8_t     is_agif;                /**< this is an agif or gif                         */
} gif_dsc_t;

/**
 * @brief  Using to define loop end call back function.
 */
typedef void (*loop_end_func)(void);
typedef gif_dsc_t agif_dsc_t;

#ifdef ANIM_LV_TASK
/**
 * @brief  Define gif description struct.
 */
typedef struct
{
    lv_img_t        img_base;
    void           *imgdsc;
    void           *gif;
    lv_timer_t     *task;
    uint16_t        task_period;
    bool            pause_rel_gif;
    uint8_t        *src_data;
    int             fd;
    uint8_t         out_bp;
    uint8_t         is_agif : 1;
    uint8_t         is_v9   : 1;
    uint16_t        delay_play_time;
    uint16_t        delay_play_num;
    uint32_t        offset;
} lv_gif_dec_t;

typedef lv_gif_dec_t lv_agif_ext_t;

/**
 * @brief  Set whether to enter loop playback after gif playback is completed.
 * @param  img gif obj.
 * @param  loop loop or not.
 * @param  interval The interval between the completion of gif playback and the next loop.
 */
void        lv_gif_dec_loop(lv_obj_t *img, bool loop, uint32_t interval);

/**
 * @brief  Attempting to open a gif, obtain gif header information, and create an lv_obj.
 *         this function also create a lv_task with PRIO_OFF, and get first frame_data
 * @param  parent parent of gif obj to be created.
 * @param  data gif src, file name or data addr.
 * @param  bg_color not used. for future.
 * @param  out_bp output bits of pixel, 24 or 16
 * @retval obj lvgl object of gif.
 */
lv_obj_t   *lv_gif_dec_create(lv_obj_t *parent, const void *data, void *bg_color, uint8_t out_bp);

/**
 * @brief  This function extends the lv_gif_dec_create by supporting reverse playback and starting playback at the specified frame number position
 * @param  parent parent of gif obj to be created.
 * @param  data gif src, file name or data addr.
 * @param  bg_color not used. for future.
 * @param  out_bp output bits of pixel, 24 or 16
 * @param  order 0: positive order; 1: reverse order
 * @param  start_frame_no 0: default order play; other: play from indicated frames.
 * @retval obj lvgl object of gif.
 */
lv_obj_t   *lv_gif_dec_create_ext(lv_obj_t *parent, const void *data, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no);

/**
 * @brief  This function extends the lv_gif_dec_create by supporting reverse playback and starting playback at the specified frame number position
 * @param  parent parent of gif obj to be created.
 * @param  data gif src, file name or data addr.
 * @param  bg_color not used. for future.
 * @param  out_bp output bits of pixel, 24 or 16
 * @param  order 0: positive order; 1: reverse order
 * @param  start_frame_no 0: default order play; other: play from indicated frames.
 * @param  cache_all 0: read for every frame; other: attempt to call all frames.
 * @retval obj lvgl object of gif.
 */
lv_obj_t *lv_gif_dec_create_comm(lv_obj_t *parent, const void *data, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no, uint8_t cache_all);

/**
 * @brief  This function extends lv_gif_dec_create_ext to supporting wf tools only.
 * @param  parent parent of gif obj to be created.
 * @param  data gif src, file name or data addr.
 * @param  offset gif begin address in data.
 * @param  bg_color not used. for future.
 * @param  out_bp output bits of pixel, 24 or 16
 * @param  order 0: positive order; 1: reverse order
 * @param  start_frame_no 0: default order play; other: play from indicated frames.
 * @param  cache_all 0: read for every frame; other: attempt to call all frames.
 * @retval obj lvgl object of gif.
 */
lv_obj_t *lv_gif_wf_dec_create(lv_obj_t *parent, const void *data, uint32_t offset, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no, uint8_t cache_all);

/**
 * @brief  This function reset gif src of img.
 * @param  img gif obj.
 * @param  data gif src, file name or data addr.
 * @param  offset gif begin address in data.
 * @retval img gif obj. it will be return NULL when decoding failed.
 */
void *lv_gif_wf_dec_resrc(lv_obj_t *img, const void *data, uint32_t offset);

/**
 * @brief  Close a gif, release gif context.
 * @param  img gif obj.
 */
void        lv_gif_dec_destroy(lv_obj_t *img);

/**
 * @brief  This function will play gif again.
 * @param  img gif obj.
 */
void        lv_gif_dec_restart(lv_obj_t *img);

/**
 * @brief  Get total gif frame number.
 * @param  img gif obj.
 * @retval frame_number.
 */
uint16_t    lv_gif_dec_frame_num(lv_obj_t *img);

/**
 * @brief  Decode next frame.
 * @param  img gif obj.
 */
int         lv_gif_dec_next_frame(lv_obj_t *img);

/**
 * @brief  Decode indicated frame.
 * @param  img gif obj.
 * @param  frame_no indicated frame no.
 */
int         lv_gif_dec_indicated_frame(lv_obj_t *img, uint16_t frame_no);

/**
 * @brief  Call back function when decoding end.
 * @param  img gif obj.
 * @param  func call back functiono.
 */
void        lv_gif_dec_end_cb_register(lv_obj_t *img, loop_end_func func);

/**
 * @brief  Create a gif decode task for img obj. This function will immediately start running this task
 * @param  img gif obj.
 * @param  peroid task period.
 * @retval frame_number.
 */
lv_timer_t *lv_gif_dec_task_create(lv_obj_t *img, uint16_t peroid);

/**
 * @brief  Pause a gif decode task of img obj
 * @param  img gif obj.
 * @param  rel_gif release gif context or not. Indicate whether to release the occupied memory resources
 */
void        lv_gif_dec_task_pause(lv_obj_t *img, bool rel_gif);

/**
 * @brief  Resume a gif decode task of img obj
 * @param  img gif obj.
 */
void        lv_gif_dec_task_resume(lv_obj_t *img);

/**
 * @brief  This function extends lv_gif_dec_task_resume to support play with delay time.
 * @param  img gif obj.
 * @param  delay_play_time resume play with delay time.
 */
void        lv_gif_dec_task_resume_with_delay(lv_obj_t *img, uint16_t delay_play_time);

/**
 * @brief  Delete a gif decode task of img obj
 * @param  img gif obj.
 */
void        lv_gif_dec_task_del(lv_obj_t *img);

#endif

/**
 * @brief  Attempting to open a gif, obtain gif header information, only for no_lvgl senario.
 * @param  data gif src, file name or data addr.
 * @param  out_bp output bits of pixel, 24 or 16
 * @retval handle gif desc handle.
 */
gif_dsc_t  *gif_dec_open(const void *data, uint8_t out_bp);

/**
 * @brief  Close a gif, release gif context.
 * @param  gif_desc gif desc handle.
 */
void        gif_dec_close(gif_dsc_t *gif_desc);

/**
 * @brief  Decode next frame.
 * @param  gif_desc gif desc handle.
 */
int         gif_dec_frame(gif_dsc_t *gif_desc);

/**
 * @brief  Decode indicated frame.
 * @param  gif_desc gif desc handle.
 * @param  frame_no indicated frame no.
 */
int         gif_dec_indicated_frame(gif_dsc_t *gif_dsc, uint16_t frame_no);

/**
 * @brief  This function will reset gif to initial state.
 * @param  gif_desc gif desc handle.
 */
void        gif_dec_reset(gif_dsc_t *gif_desc);

/**
 * @brief  Get total gif frame number.
 * @param  gif_desc gif desc handle.
 * @retval frame_number.
 */
uint16_t    gif_dec_frame_num(gif_dsc_t *gif_dsc);

/**
 * @brief  Not used.
 */
void        gif_set_fs_mode(agif_fs_mode_t mode);

void       *gif_alloc(size_t size);
void       *gif_realloc(void *rmem, rt_size_t newsize);
void        gif_free(void *ptr);
int         gif_fopen(const char *file, int flags);
int         gif_fclose(int fd);
int         gif_fread(int fd, void *buf, size_t len);
off_t       gif_lseek(int fd, off_t offset, int whence);
int         gif_ioctl(int fd, int mode, uint32_t *addr);

/**
 * @brief  The following interfaces are for internal use only
 */
#ifdef ANIM_LV_TASK
void        _lv_gif_dec_loop(lv_obj_t *img, bool loop, uint32_t interval);
void        _lv_gif_dec_loop_ext(lv_obj_t *img, uint32_t interval);
lv_obj_t   *_lv_gif_dec_create(lv_obj_t *parent, const void *data,  void *bg_color, uint8_t out_bp);
lv_obj_t   *_lv_gif_dec_create_ext(lv_obj_t *parent, const void *data, uint32_t offset, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no);
lv_obj_t   *_lv_gif_dec_resrc(lv_obj_t *img, const void *data, uint32_t offset);
void        _lv_gif_dec_restart(lv_obj_t *img);
uint16_t    _lv_gif_dec_frame_num(lv_obj_t *img);
lv_timer_t *_lv_gif_dec_task_create(lv_obj_t *img, uint16_t peroid);
void        _lv_gif_dec_task_pause(lv_obj_t *img, bool rel_gif);
void        _lv_gif_dec_task_resume_with_delay(lv_obj_t *img, uint16_t delay_play_time);
void        _lv_gif_dec_task_resume(lv_obj_t *img);
void        _lv_gif_dec_task_del(lv_obj_t *img);
void        _lv_gif_dec_destroy(lv_obj_t *img);
int         _lv_gif_dec_next_frame(lv_obj_t *anim);
int         _lv_gif_dec_indicated_frame(lv_obj_t *img, uint16_t frame_no);
void        _lv_gif_dec_end_cb_register(lv_obj_t *img, loop_end_func func);

void         lv_agif_dec_loop(lv_obj_t *img, bool loop, uint32_t interval);
void         lv_agif_dec_loop_ext(lv_obj_t *img, uint32_t interval);
lv_obj_t    *lv_agif_dec_create_comm(lv_obj_t *parent, const void *data, uint32_t offset, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no, uint8_t cache_all);
lv_obj_t    *lv_agif_dec_create(lv_obj_t *parent, const void *data, void *bg_color, uint8_t out_bp);
lv_obj_t    *lv_agif_dec_resrc(lv_obj_t *img, const void *data, uint32_t offset);
lv_obj_t    *lv_agif_dec_create_ext(lv_obj_t *parent, const void *data, uint32_t offset, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no);
lv_obj_t    *lv_agif_wf_dec_create(lv_obj_t *parent, const void *data, uint32_t offset, void *bg_color, uint8_t out_bp);

void         lv_agif_dec_restart(lv_obj_t *agif);
lv_timer_t  *lv_agif_dec_task_create(lv_obj_t *img, uint16_t period);
void         lv_agif_dec_task_pause(lv_obj_t *img, bool rel_gif);
void         lv_agif_dec_task_resume_with_delay(lv_obj_t *img, uint16_t delay_play_time);
void         lv_agif_dec_task_resume(lv_obj_t *img);
void         lv_agif_dec_task_del(lv_obj_t *img);
void         lv_agif_dec_destroy(lv_obj_t *img);
int          lv_agif_dec_next_frame(lv_obj_t *agif);
int          lv_agif_dec_indicated_frame(lv_obj_t *img, uint16_t frame_no);
uint16_t     lv_agif_dec_frame_num(lv_obj_t *img);
void         lv_agif_dec_end_cb_register(lv_obj_t *img, loop_end_func func);
#else
extern void *lv_img_class;
void        *lv_disp_get_default(void);
void         lv_img_set_src(void *obj, void *src);
void        *lv_obj_class_create_obj(void *class_p, void *parent);
void         lv_obj_class_init_obj(void *obj);
void         lv_obj_del(void *obj);
void         lv_obj_invalidate(void *obj);
uint32_t     lv_tick_get(void);
void        *lv_timer_create(void *timer_xcb, uint32_t period, void *user_data);
void         lv_timer_del(void *timer);
void         lv_timer_pause(void *timer);
void         lv_timer_resume(void *timer);
void         lv_timer_set_period(void *timer, uint32_t period);
void         lv_img_cache_invalidate_src(void *src);
#endif

uint32_t    _gif_builtin_flash_read(uint32_t addr, uint8_t *buf, int size);

gif_dsc_t  *_gif_dec_open(const void *data, uint8_t out_bp);
void        _gif_dec_close(gif_dsc_t *gif_desc);
int         _gif_dec_frame(gif_dsc_t *gif_desc);
int         _gif_dec_indicated_frame(gif_dsc_t *gif_dsc, uint16_t frame_no);
void        _gif_dec_set_mem_type(uint8_t mem_type);
void        _gif_dec_reset(gif_dsc_t *gif_desc);
uint16_t    _gif_dec_frame_num(gif_dsc_t *gif_dsc);
void       *_gif_dec_mem_alloc(rt_size_t size, uint8_t *mem_type);
void        _gif_dec_mem_free(void *ptr, uint8_t mem_type);


agif_dsc_t  *agif_dec_open(const void *data, uint8_t out_bp);
agif_dsc_t  *agif_dec_open_ext(const void *data, uint8_t out_bp, uint8_t order, uint16_t start_frame_no);
void         agif_dec_close(agif_dsc_t *agif_desc);
int          agif_dec_frame(agif_dsc_t *agif_desc);
int          agif_dec_indicated_frame(agif_dsc_t *agif_dsc, uint16_t frame_no);
void         agif_dec_reset(agif_dsc_t *agif_desc);
uint16_t     agif_dec_frame_num(agif_dsc_t *agif_dsc);
void         agif_set_fs_mode(agif_fs_mode_t mode);

#ifdef __cplusplus
}
#endif
#endif /* _AGIF_H_ */

