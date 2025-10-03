/*
 * SPDX-FileCopyrightText: 2019-2024 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtthread.h"

int gif_log = 0;

#if !defined (BF0_LCPU)

#ifdef RT_USING_FINSH
#include <finsh.h>
int gif_log_log(void)
{
    gif_log = (gif_log + 1) & 0x01;
    rt_kprintf("anim_mem_log: %d\n", gif_log);
    return 0;
}
MSH_CMD_EXPORT_ALIAS(gif_log_log, gif_log, gif_log: open or close gif_log log);
#endif

#include "agif.h"

#ifdef RT_USING_DFS
    #include <dfs_posix.h>
#endif

#if !defined (BSP_USING_PC_SIMULATOR)
    #define AGIF_SUPPORT
#endif

#ifdef ANIM_LV_TASK
void lv_gif_dec_loop(lv_obj_t *img, bool loop, uint32_t interval)
{
#ifdef AGIF_SUPPORT
    lv_agif_dec_loop(img, loop, interval);
#else
    _lv_gif_dec_loop(img, loop, interval);
#endif
}

lv_obj_t *lv_gif_dec_create(lv_obj_t *parent, const void *data, void *bg_color, uint8_t out_bp)
{
#ifdef LVGL_V9
    out_bp = 0x80 | out_bp;
#endif
#ifdef AGIF_SUPPORT
    return lv_agif_dec_create(parent, data, bg_color, out_bp);
#else
    return _lv_gif_dec_create(parent, data, bg_color, out_bp);
#endif
}

lv_obj_t *lv_gif_dec_create_ext(lv_obj_t *parent, const void *data, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no)
{
#ifdef LVGL_V9
    out_bp = 0x80 | out_bp;
#endif
#ifdef AGIF_SUPPORT
    return lv_agif_dec_create_ext(parent, data, 0, bg_color, out_bp, order, start_frame_no);
#else
    return _lv_gif_dec_create_ext(parent, data, 0, bg_color, out_bp, order, start_frame_no);
#endif
}

lv_obj_t *lv_gif_dec_create_comm(lv_obj_t *parent, const void *data, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no, uint8_t cache_all)
{
#ifdef LVGL_V9
    out_bp = 0x80 | out_bp;
#endif
#ifdef AGIF_SUPPORT
    return lv_agif_dec_create_comm(parent, data, 0, bg_color, out_bp, order, start_frame_no, cache_all);
#else
    return _lv_gif_dec_create_ext(parent, data, 0, bg_color, out_bp, order, start_frame_no);
#endif
}

lv_obj_t *lv_gif_wf_dec_create(lv_obj_t *parent, const void *data, uint32_t offset, void *bg_color, uint8_t out_bp, uint8_t order, uint16_t start_frame_no, uint8_t cache_all)
{
#ifdef LVGL_V9
    out_bp = 0x80 | out_bp;
#endif
#ifdef AGIF_SUPPORT
    return lv_agif_dec_create_comm(parent, data, offset, bg_color, out_bp, order, start_frame_no, cache_all);
#else
    return _lv_gif_dec_create_ext(parent, data, offset, bg_color, out_bp, order, start_frame_no);
#endif
}

void *lv_gif_wf_dec_resrc(lv_obj_t *img, const void *data, uint32_t offset)
{
#ifdef AGIF_SUPPORT
    return lv_agif_dec_resrc(img, data, offset);
#else
    return _lv_gif_dec_resrc(img, data, offset);
#endif
}

void lv_gif_dec_restart(lv_obj_t *img)
{
#ifdef AGIF_SUPPORT
    lv_agif_dec_restart(img);
#else
    _lv_gif_dec_restart(img);
#endif
}

uint16_t lv_gif_dec_frame_num(lv_obj_t *img)
{
#ifdef AGIF_SUPPORT
    return lv_agif_dec_frame_num(img);
#else
    return _lv_gif_dec_frame_num(img);
#endif
}

lv_timer_t *lv_gif_dec_task_create(lv_obj_t *img, uint16_t peroid)
{
#ifdef AGIF_SUPPORT
    return lv_agif_dec_task_create(img, peroid);
#else
    return _lv_gif_dec_task_create(img, peroid);
#endif
}

void lv_gif_dec_task_pause(lv_obj_t *img, bool rel_gif)
{
#ifdef AGIF_SUPPORT
    lv_agif_dec_task_pause(img, rel_gif);
#else
    _lv_gif_dec_task_pause(img, rel_gif);
#endif
}

void lv_gif_dec_task_resume_with_delay(lv_obj_t *img, uint16_t delay_play_time)
{
#ifdef AGIF_SUPPORT
    lv_agif_dec_task_resume_with_delay(img, delay_play_time);
#else
    _lv_gif_dec_task_resume_with_delay(img, delay_play_time);
#endif
}

void lv_gif_dec_task_resume(lv_obj_t *img)
{
#ifdef AGIF_SUPPORT
    lv_agif_dec_task_resume(img);
#else
    _lv_gif_dec_task_resume(img);
#endif
}

void lv_gif_dec_task_del(lv_obj_t *img)
{
#ifdef AGIF_SUPPORT
    return lv_agif_dec_task_del(img);
#else
    _lv_gif_dec_task_del(img);
#endif
}

void lv_gif_dec_destroy(lv_obj_t *img)
{
#ifdef AGIF_SUPPORT
    lv_agif_dec_destroy(img);
#else
    _lv_gif_dec_destroy(img);
#endif
}

int  lv_gif_dec_next_frame(lv_obj_t *img)
{
#ifdef AGIF_SUPPORT
    return lv_agif_dec_next_frame(img);
#else
    return _lv_gif_dec_next_frame(img);
#endif
}

int  lv_gif_dec_indicated_frame(lv_obj_t *img, uint16_t frame_no)
{
#ifdef AGIF_SUPPORT
    return lv_agif_dec_indicated_frame(img, frame_no);
#else
    return _lv_gif_dec_indicated_frame(img, frame_no);
#endif
}

void lv_gif_dec_end_cb_register(lv_obj_t *img, loop_end_func func)
{
#ifdef AGIF_SUPPORT
    lv_agif_dec_end_cb_register(img, func);
#else
    _lv_gif_dec_end_cb_register(img, func);
#endif
}

#else
void  lv_img_set_src(void *obj, void *src) {}
void  lv_obj_class_init_obj(void *obj) {}
void  lv_obj_del(void *obj) {}
void  lv_obj_invalidate(void *obj) {}
void  lv_timer_del(void *timer) {}
void  lv_timer_pause(void *timer) {}
void  lv_timer_resume(void *timer) {}
void  lv_timer_set_period(void *timer, uint32_t period) {}
void  lv_img_cache_invalidate_src(void *src) {}
void *lv_img_class;
void *lv_disp_get_default(void)
{
    return NULL;
}
void *lv_obj_class_create_obj(void *class_p, void *parent)
{
    return NULL;
}
void *lv_timer_create(void *timer_xcb, uint32_t period, void *user_data)
{
    return NULL;
}

uint32_t lv_tick_get(void)
{
    return 0;
}
#endif

uint32_t _gif_builtin_flash_read(uint32_t addr, uint8_t *buf, int size)
{
#if 0//def ANIM_LV_TASK 
    return lv_img_decode_flash_read(addr, buf, size);
#else
    return 0;
#endif
}

gif_dsc_t *gif_dec_open(const void *data, uint8_t out_bp)
{
#ifdef AGIF_SUPPORT
    return agif_dec_open(data, out_bp);
#else
    return _gif_dec_open(data, out_bp);
#endif
}

void gif_dec_close(gif_dsc_t *gif_dsc)
{
#ifdef AGIF_SUPPORT
    agif_dec_close(gif_dsc);
#else
    _gif_dec_close(gif_dsc);
#endif
}

int  gif_dec_frame(gif_dsc_t *gif_dsc)
{
#ifdef AGIF_SUPPORT
    return agif_dec_frame(gif_dsc);
#else
    return _gif_dec_frame(gif_dsc);
#endif
}

int  gif_dec_indicated_frame(gif_dsc_t *gif_dsc, uint16_t frame_no)
{
#ifdef AGIF_SUPPORT
    return agif_dec_indicated_frame(gif_dsc, frame_no);
#else
    return _gif_dec_indicated_frame(gif_dsc, frame_no);
#endif
}

void gif_dec_reset(gif_dsc_t *gif_dsc)
{
#ifdef AGIF_SUPPORT
    agif_dec_reset(gif_dsc);
#else
    _gif_dec_reset(gif_dsc);
#endif
}

uint16_t gif_dec_frame_num(gif_dsc_t *gif_dsc)
{
#ifdef AGIF_SUPPORT
    return agif_dec_frame_num(gif_dsc);
#else
    return _gif_dec_frame_num(gif_dsc);
#endif
}


void gif_set_fs_mode(agif_fs_mode_t mode)
{
#ifdef AGIF_SUPPORT
    agif_set_fs_mode(mode);
#else
#endif
}

void *gif_alloc(size_t size)
{
#if 1//defined (DFU_OTA_MANAGER)
    return rt_malloc(size);
#else
    return app_cache_alloc(size, IMAGE_CACHE_PSRAM);
#endif
}

void *gif_realloc(void *rmem, rt_size_t newsize)
{
    if (0 == newsize)
    {
        gif_free(rmem);
        return NULL;
    }

#if 1//defined (DFU_OTA_MANAGER)
    return rt_realloc(rmem, newsize);
#else
    void *ptr = app_cache_alloc(newsize, IMAGE_CACHE_PSRAM);
    if (ptr)
    {
        memcpy(ptr, rmem, newsize);
    }
    gif_free(rmem);
    return ptr;
#endif
}

void gif_free(void *ptr)
{
#if 1//defined (DFU_OTA_MANAGER)
    rt_free(ptr);
#else
    app_cache_free(ptr);
#endif
}

int gif_fopen(const char *file, int flags)
{
#ifdef RT_USING_DFS
#ifdef WIN32
    return rt_open(file, flags, 0x644);
#else
    return open(file, flags);
#endif
#else
    return -1;
#endif
}

int gif_fclose(int fd)
{
#ifdef RT_USING_DFS
#ifdef WIN32
    return rt_close(fd);
#else
    return close(fd);
#endif
#else
    return -1;
#endif
}

int gif_fread(int fd, void *buf, size_t len)
{
#ifdef RT_USING_DFS
#ifdef WIN32
    return rt_read(fd, buf, len);
#else
    return read(fd, buf, len);
#endif
#else
    return 0;
#endif
}

off_t gif_lseek(int fd, off_t offset, int whence)
{
#ifdef RT_USING_DFS
#ifdef WIN32
    return rt_lseek(fd, offset, whence);
#else
    return lseek(fd, offset, whence);
#endif
#else
    return -1;
#endif
}

int gif_ioctl(int fd, int mode, uint32_t *addr)
{
#ifdef RT_USING_DFS
    return ioctl(fd, mode, addr);
#else
    return -1;
#endif
}

#endif
