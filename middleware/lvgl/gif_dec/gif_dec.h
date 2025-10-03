/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __GIF_DEC_H__
#define __GIF_DEC_H__
#include "stdint.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

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

typedef void (*loop_end_func)(void);
typedef gif_dsc_t agif_dsc_t;

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

#ifdef __cplusplus
}
#endif

#endif /* __GIF_DEC_H__ */

