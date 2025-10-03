/*
 * default memory allocator for libavutil
 * Copyright (c) 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * default memory allocator for libavutil
 */

#define _XOPEN_SOURCE 600

#include "config.h"
#include "rtthread.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_MALLOC_H
//#include <malloc.h>
#endif
#include "internal.h"
#include "avassert.h"
#include "avutil.h"
#include "common.h"
#include "dynarray.h"
#include "intreadwrite.h"
#include "mem.h"

extern void *ffmpeg_alloc(size_t nbytes);
extern void *ffmpeg_realloc(void *p, size_t new_size);
extern void ffmpeg_free(void *ptr);


#ifdef WIN32
#define ENOMEM 12
#endif

#include "mem_internal.h"

#define ALIGN (HAVE_AVX ? 32 : 16)

/* NOTE: if you want to override these functions with your own
 * implementations (not recommended) you have to link libav* as
 * dynamic libraries and remove -Wl,-Bsymbolic from the linker flags.
 * Note that this will cost performance. */

static size_t max_alloc_size= INT_MAX;


void av_max_alloc(size_t max){
    max_alloc_size = max;
}
#if FFMPEG_MEM_DEBUG

typedef struct {
    struct rt_list_node node;
    const char *filename;
    int line;
    size_t size;
    uint8_t malloc_mem[0];
} ffmpeg_mem_node;

static struct rt_list_node root;

void ffmeg_mem_init()
{
    int ret = AVERROR(ENOMEM);

    av_assert0(ret < 0);

    rt_list_init(&root);
}
//not thread safe, call after meida server exit or call in vido thread
void ffmpeg_memleak_check()
{
    rt_list_t *pos;
    ffmpeg_mem_node *node;
    if (rt_list_isempty(&root))
    {
        rt_kprintf("ffmpeg mem no leak\r\n");
        return;
    }
    rt_kprintf("ffmpeg mem leak:\r\n");
    rt_list_for_each(pos, &root)
    {
        node = rt_list_entry(pos, ffmpeg_mem_node, node);
        rt_kprintf("%s line %d size %d\r\n", node->filename, node->line, node->size);
    }
}
#else
void ffmeg_mem_init()
{
}

void ffmpeg_memleak_check()
{
}
#endif

#if FFMPEG_MEM_DEBUG
void *av_malloc_do(size_t size, const char *file, int line)
{
    ffmpeg_mem_node *ptr = NULL;
    /* let's disallow possibly ambiguous cases */
    if (size > (max_alloc_size - 32) || size == 0)
        return NULL;

    ptr = (ffmpeg_mem_node*)ffmpeg_alloc(sizeof(ffmpeg_mem_node) + size);
    if (!ptr)
    {
        rt_kprintf("av_malloc_do: %s line %d size=%d\r\n", file, line, size);
        av_assert0(ptr);
    }
    else
    {
        ptr->filename = file;
        ptr->line = line;
        ptr->size = size;
        rt_enter_critical();
        rt_list_insert_before(&root, &ptr->node);
        rt_exit_critical();
    }

    return (void*)ptr->malloc_mem;
}

void *av_realloc_do(void *ptr, size_t size, const char *file, int line)
{
    ffmpeg_mem_node *new_node;
    if (size > (max_alloc_size - 32))
        return NULL;
    if (!size && !ptr)
    {
        return NULL;
    }
    if (!ptr) // size > 0
        return av_malloc_do(size, file, line);

    // now ptr != 0
    if (size == 0)
    {
        av_free_do(ptr, file, line);
        return NULL;
    }
    //now  ptr!=0 && size != 0
    void *new_ptr = av_malloc_do(size, file, line);
    if (new_ptr)
    {
        memcpy(new_ptr, ptr, size);
        av_free_do(ptr, file, line);
    }

    return new_ptr;
}

void *av_realloc_f_do(void *ptr, size_t nelem, size_t elsize, const char *file, int line)
{
    size_t size;
    void *r;

    if (av_size_mult(elsize, nelem, &size)) {
        av_free_do(ptr, file, line);
        return NULL;
    }
    r = av_realloc_do(ptr, size, file, line);
    if (!r && size)
        av_free_do(ptr, file, line);
    return r;
}
int av_reallocp_do(void *ptr, size_t size, const char *file, int line)
{
    void *val;

    if (!size) {
        av_freep_do(ptr, file, line);
        return 0;
    }

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc_do(val, size, file, line);

    if (!val) {
        //av_freep(ptr); has freed in av_realloc_do()
        memcpy(ptr, &val, sizeof(val));
        return AVERROR(ENOMEM);
    }

    memcpy(ptr, &val, sizeof(val));
    return 0;
}
void av_free_do(void *ptr, const char *file, int line)
{
    ffmpeg_mem_node *node;
    if (ptr)
    {
        node = rt_container_of(ptr, ffmpeg_mem_node, malloc_mem);
        rt_enter_critical();
        rt_list_remove(&node->node);
        rt_exit_critical();
        ffmpeg_free(node);
    }
}
void *av_mallocz_do(size_t size, const char *file, int line)
{
    if (!size || size >= INT_MAX/4)
        return NULL;

    void *ptr = av_malloc_do(size, file, line);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}
char *av_strdup_do(const char *s, const char *file, int line)
{
    char *ptr = NULL;
    if (s) {
        size_t len = strlen(s) + 1;
        ptr = av_realloc_do(NULL, len, file, line);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}
char *av_strndup_do(const char *s, size_t len, const char *file, int line)
{
    char *ret = NULL, *end;

    if (!s)
        return NULL;

    end = memchr(s, 0, len);
    if (end)
        len = end - s;

    ret = av_realloc_do(NULL, len + 1, file, line);
    if (!ret)
        return NULL;

    memcpy(ret, s, len);
    ret[len] = 0;
    return ret;
}
void *av_memdup_do(const void *p, size_t size, const char *file, int line)
{
    void *ptr = NULL;
    if (p) {
        ptr = av_malloc_do(size, file, line);
        if (ptr)
            memcpy(ptr, p, size);
    }
    return ptr;
}
void av_freep_do(void *ptr, const char *file, int line)
{
    void *val;

    memcpy(&val, ptr, sizeof(val));
    memset(ptr, 0, sizeof(val));
    av_free_do(val, file, line);
}
#else

void *av_malloc(size_t size)
{
    void *ptr = NULL;
    /* let's disallow possibly ambiguous cases */
    if (size > (max_alloc_size - 32) || size == 0)
        return NULL;

    ptr = ffmpeg_alloc(size + !size);
    if (!ptr)
    {
        rt_kprintf("av_malloc size=%d\r\n", size);
        av_assert0(ptr);
    }
    return ptr;
}

void *av_realloc(void *ptr, size_t size)
{
    /* let's disallow possibly ambiguous cases */
    if (size > (max_alloc_size - 32))
        return NULL;
    if (!size && !ptr)
    {
        return NULL;
    }
    if (!ptr)
        return av_malloc(size);
    if (size == 0)
    {
        ffmpeg_free(ptr);
        return NULL;
    }
#if 1
    return ffmpeg_realloc(ptr, size);
#else
    void *new_ptr = ffmpeg_alloc(size);

    if (new_ptr)
    {
        memcpy(new_ptr, ptr, size);
        ffmpeg_free(ptr);
    }
    else
    {
        //could not free ptr, see av_realloc_f & av_reallocp
    }
    return new_ptr;
#endif
}

void *av_realloc_f(void *ptr, size_t nelem, size_t elsize)
{
    size_t size;
    void *r;

    if (av_size_mult(elsize, nelem, &size)) {
        av_free(ptr);
        return NULL;
    }
    r = av_realloc(ptr, size);
    if (!r && size)
        av_free(ptr);
    return r;
}

int av_reallocp(void *ptr, size_t size)
{
    void *val;

    if (!size) {
        av_freep(ptr);
        return 0;
    }

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc(val, size);

    if (!val) {
        av_freep(ptr);
        return AVERROR(ENOMEM);
    }

    memcpy(ptr, &val, sizeof(val));
    return 0;
}

void *av_realloc_array(void *ptr, size_t nmemb, size_t size)
{
    if (!size || nmemb >= INT_MAX / size)
        return NULL;
    return av_realloc(ptr, nmemb * size);
}

int av_reallocp_array(void *ptr, size_t nmemb, size_t size)
{
    void *val;

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc_f(val, nmemb, size);
    memcpy(ptr, &val, sizeof(val));
    if (!val && nmemb && size)
        return AVERROR(ENOMEM);

    return 0;
}

void av_free(void *ptr)
{
    ffmpeg_free(ptr);
}

void av_freep(void *arg)
{
    void *val;

    memcpy(&val, arg, sizeof(val));
    memcpy(arg, &(void *){ NULL }, sizeof(val));
    av_free(val);
}

void *av_mallocz(size_t size)
{
    void *ptr = av_malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

void *av_calloc(size_t nmemb, size_t size)
{
    if (size <= 0 || nmemb >= INT_MAX / size)
        return NULL;
    return av_mallocz(nmemb * size);
}

char *av_strdup(const char *s)
{
    char *ptr = NULL;
    if (s) {
        size_t len = strlen(s) + 1;
        ptr = av_realloc(NULL, len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

char *av_strndup(const char *s, size_t len)
{
    char *ret = NULL, *end;

    if (!s)
        return NULL;

    end = memchr(s, 0, len);
    if (end)
        len = end - s;

    ret = av_realloc(NULL, len + 1);
    if (!ret)
        return NULL;

    memcpy(ret, s, len);
    ret[len] = 0;
    return ret;
}

void *av_memdup(const void *p, size_t size)
{
    void *ptr = NULL;
    if (p) {
        ptr = av_malloc(size);
        if (ptr)
            memcpy(ptr, p, size);
    }
    return ptr;
}
#endif
int av_dynarray_add_nofree(void *tab_ptr, int *nb_ptr, void *elem)
{
    void **tab;
    memcpy(&tab, tab_ptr, sizeof(tab));

    AV_DYNARRAY_ADD(INT_MAX, sizeof(*tab), tab, *nb_ptr, {
        tab[*nb_ptr] = elem;
        memcpy(tab_ptr, &tab, sizeof(tab));
    }, {
        return AVERROR(ENOMEM);
    });
    return 0;
}

void av_dynarray_add(void *tab_ptr, int *nb_ptr, void *elem)
{
    void **tab;
    memcpy(&tab, tab_ptr, sizeof(tab));

    AV_DYNARRAY_ADD(INT_MAX, sizeof(*tab), tab, *nb_ptr, {
        tab[*nb_ptr] = elem;
        memcpy(tab_ptr, &tab, sizeof(tab));
    }, {
        *nb_ptr = 0;
        av_freep(tab_ptr);
    });
}

void *av_dynarray2_add(void **tab_ptr, int *nb_ptr, size_t elem_size,
                       const uint8_t *elem_data)
{
    uint8_t *tab_elem_data = NULL;

    AV_DYNARRAY_ADD(INT_MAX, elem_size, *tab_ptr, *nb_ptr, {
        tab_elem_data = (uint8_t *)*tab_ptr + (*nb_ptr) * elem_size;
        if (elem_data)
            memcpy(tab_elem_data, elem_data, elem_size);
        else if (CONFIG_MEMORY_POISONING)
            memset(tab_elem_data, FF_MEMORY_POISON, elem_size);
    }, {
        av_freep(tab_ptr);
        *nb_ptr = 0;
    });
    return tab_elem_data;
}

static void fill16(uint8_t *dst, int len)
{
    uint32_t v = AV_RN16(dst - 2);

    v |= v << 16;

    while (len >= 4) {
        AV_WN32(dst, v);
        dst += 4;
        len -= 4;
    }

    while (len--) {
        *dst = dst[-2];
        dst++;
    }
}

static void fill24(uint8_t *dst, int len)
{
#if HAVE_BIGENDIAN
    uint32_t v = AV_RB24(dst - 3);
    uint32_t a = v << 8  | v >> 16;
    uint32_t b = v << 16 | v >> 8;
    uint32_t c = v << 24 | v;
#else
    uint32_t v = AV_RL24(dst - 3);
    uint32_t a = v       | v << 24;
    uint32_t b = v >> 8  | v << 16;
    uint32_t c = v >> 16 | v << 8;
#endif

    while (len >= 12) {
        AV_WN32(dst,     a);
        AV_WN32(dst + 4, b);
        AV_WN32(dst + 8, c);
        dst += 12;
        len -= 12;
    }

    if (len >= 4) {
        AV_WN32(dst, a);
        dst += 4;
        len -= 4;
    }

    if (len >= 4) {
        AV_WN32(dst, b);
        dst += 4;
        len -= 4;
    }

    while (len--) {
        *dst = dst[-3];
        dst++;
    }
}

static void fill32(uint8_t *dst, int len)
{
    uint32_t v = AV_RN32(dst - 4);

    while (len >= 4) {
        AV_WN32(dst, v);
        dst += 4;
        len -= 4;
    }

    while (len--) {
        *dst = dst[-4];
        dst++;
    }
}

void av_memcpy_backptr(uint8_t *dst, int back, int cnt)
{
    const uint8_t *src = &dst[-back];
    if (!back)
        return;

    if (back == 1) {
        memset(dst, *src, cnt);
    } else if (back == 2) {
        fill16(dst, cnt);
    } else if (back == 3) {
        fill24(dst, cnt);
    } else if (back == 4) {
        fill32(dst, cnt);
    } else {
        if (cnt >= 16) {
            int blocklen = back;
            while (cnt > blocklen) {
                memcpy(dst, src, blocklen);
                dst       += blocklen;
                cnt       -= blocklen;
                blocklen <<= 1;
            }
            memcpy(dst, src, cnt);
            return;
        }
        if (cnt >= 8) {
            AV_COPY32U(dst,     src);
            AV_COPY32U(dst + 4, src + 4);
            src += 8;
            dst += 8;
            cnt -= 8;
        }
        if (cnt >= 4) {
            AV_COPY32U(dst, src);
            src += 4;
            dst += 4;
            cnt -= 4;
        }
        if (cnt >= 2) {
            AV_COPY16U(dst, src);
            src += 2;
            dst += 2;
            cnt -= 2;
        }
        if (cnt)
            *dst = *src;
    }
}

void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size)
{
    if (min_size < *size)
        return ptr;

    min_size = FFMAX(min_size + min_size / 16 + 32, min_size);

    ptr = av_realloc(ptr, min_size);
    /* we could set this to the unmodified min_size but this is safer
     * if the user lost the ptr and uses NULL now
     */
    if (!ptr)
        min_size = 0;

    *size = min_size;

    return ptr;
}

void av_fast_malloc(void *ptr, unsigned int *size, size_t min_size)
{
    ff_fast_malloc(ptr, size, min_size, 0);
}

void av_fast_mallocz(void *ptr, unsigned int *size, size_t min_size)
{
    ff_fast_malloc(ptr, size, min_size, 1);
}
