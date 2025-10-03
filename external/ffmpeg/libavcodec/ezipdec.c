/**
 * @file
 * LCL (LossLess Codec Library) Video Codec
 * Decoder for EZIP codecs
 * Experimental encoder for ZLIB RGB24
 *
 * Fourcc: ezi5, ezi8

 */

#include <stdio.h>
#include <stdlib.h>

#include "libavutil/avassert.h"
#include "avcodec.h"
#include "internal.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

typedef struct ezipDecContext {
    int is_5565;
} ezipDecContext;

extern void rt_kprintf(const char * fmt, ...);
static int decode_frame(AVCodecContext *avctx, AVFrame *frame,
                        int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    ezipDecContext * const c = avctx->priv_data;
    int ret;
    unsigned char *encoded = avpkt->data, *outptr;
    unsigned int width = avctx->width; // Real image width
    unsigned int height = avctx->height; // Real image height

    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;

    uint8_t *head = frame->data[0];

    memcpy(head, &buf_size, sizeof(int));
    memcpy(head + sizeof(int), buf, buf_size);

    /* Decompress frame */
    switch (avctx->codec_id) {
    case AV_CODEC_ID_EZIP565:
        break;
    case AV_CODEC_ID_EZIP888:
            break;
    }
    frame->key_frame = 1;
    frame->pict_type = AV_PICTURE_TYPE_I;

    *got_frame = 1;

    /* always report that the buffer was completely consumed */
    return buf_size;
}

static av_cold int decode_init_565(AVCodecContext *avctx)
{
    rt_kprintf("decode_init_565\n");
    ezipDecContext * const c = avctx->priv_data;
    avctx->pix_fmt = AV_PIX_FMT_RGB565LE;
    c->is_5565 = 1;
    return 0;
}

static av_cold int decode_init_888(AVCodecContext *avctx)
{
    rt_kprintf("decode_init_888\n");
    ezipDecContext * const c = avctx->priv_data;
    avctx->pix_fmt = AV_PIX_FMT_RGB565LE;
    c->is_5565 = 0;
    return 0;
}
static av_cold int decode_end(AVCodecContext *avctx)
{
    return 0;
}

AVCodec ff_ezip888_decoder = {
    .name         = "ezip888",
    .long_name    = NULL_IF_CONFIG_SMALL("LCL (LossLess Codec Library) EZIP888"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_EZIP888,
    .priv_data_size = sizeof(ezipDecContext),
    .capabilities = AV_CODEC_CAP_DR1,

    .init           = decode_init_888,
    .close          = decode_end,
    .decode         = decode_frame,
};



AVCodec ff_ezip565_decoder = {
    .name         = "ezip",
    .long_name    = NULL_IF_CONFIG_SMALL("LCL (LossLess Codec Library) EZIP565"),
    .type         = AVMEDIA_TYPE_VIDEO,
    .id           = AV_CODEC_ID_EZIP565,
    .priv_data_size = sizeof(ezipDecContext),
    .capabilities = AV_CODEC_CAP_DR1,
    .init           = decode_init_565,
    .close          = decode_end,
    .decode         = decode_frame,
};
