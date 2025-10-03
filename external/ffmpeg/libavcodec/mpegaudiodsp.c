/*
 * Copyright (c) 2011 Mans Rullgard
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

#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/thread.h"
#include "mpegaudiodsp.h"
#include "dct.h"
#include "dct32.h"

static AVOnce mpadsp_float_table_init = AV_ONCE_INIT;
static AVOnce mpadsp_fixed_table_init = AV_ONCE_INIT;

#if CONFIG_MP3FLOAT_DECODER && !CONFIG_MP3_DECODER
static void dummy_blocks_fixed(int *out, int *buf, int *in, int count, int switch_point, int block_type)
{
    av_assert0(0);
}
static void dummy_apply_window_fixed(int32_t *synth_buf, int32_t *window, int *dither_state, int16_t *samples, int incr)
{
    av_assert0(0);
}
#endif
#if !CONFIG_MP3FLOAT_DECODER && CONFIG_MP3_DECODER
void dummy_blocks_float(float *out, float *buf, float *in, int count, int switch_point, int block_type);
{
    av_assert0(0);
}
static void dummy_apply_window_float(float *synth_buf, float *window, int *dither_state, float *samples, int incr)
{
    av_assert0(0);
}
#endif

static void dummy_dct32_float(float *dst, const float *src)
{
    av_assert0(0);
}
static void dummy_dct32_fixed(int *dst, const int *src)
{
    av_assert0(0);
}

av_cold void ff_mpadsp_init(MPADSPContext *s)
{
    DCTContext dct;

    ff_dct_init(&dct, 5, DCT_II);

#if CONFIG_MP3FLOAT_DECODER
    ff_thread_once(&mpadsp_float_table_init, &ff_init_mpadsp_tabs_float);
    s->apply_window_float = ff_mpadsp_apply_window_float;
    s->dct32_float = dct.dct32;
    s->imdct36_blocks_float = ff_imdct36_blocks_float;
#if !CONFIG_MP3_DECODER
    s->apply_window_fixed = dummy_apply_window_fixed;
    s->dct32_fixed = dummy_dct32_fixed;
    s->imdct36_blocks_fixed = dummy_blocks_fixed;
#endif
#endif
#if CONFIG_MP3_DECODER
    ff_thread_once(&mpadsp_fixed_table_init, &ff_init_mpadsp_tabs_fixed);
    s->apply_window_fixed = ff_mpadsp_apply_window_fixed;
    s->dct32_fixed = ff_dct32_fixed;
    s->imdct36_blocks_fixed = ff_imdct36_blocks_fixed;
#if !CONFIG_MP3FLOAT_DECODER
    s->apply_window_float = dummy_apply_window_float;
    s->dct32_float = dummy_dct32_float;
    s->imdct36_blocks_float = dummy_blocks_float;
#endif
#endif

#if 0
    if (ARCH_AARCH64) ff_mpadsp_init_aarch64(s);
    if (ARCH_ARM)     ff_mpadsp_init_arm(s);
    if (ARCH_PPC)     ff_mpadsp_init_ppc(s);
    if (ARCH_X86)     ff_mpadsp_init_x86(s);
    if (HAVE_MIPSFPU)   ff_mpadsp_init_mipsfpu(s);
    if (HAVE_MIPSDSP) ff_mpadsp_init_mipsdsp(s);
#endif
}
