/**
 * @file
 * Decoder fore ezipP codecs
 *
 * Fourcc: ezip
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "libavutil/avassert.h"
#include "avcodec.h"
#include "codec_internal.h"
#include "encode.h"
#include "lcl.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"
#include "libavutil/imgutils.h"
typedef struct ezipEncContext {

    AVCodecContext *avctx;
    int compression;
    int is_565;
} ezipEncContext;

static int encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                        const AVFrame *p, int *got_packet)
{
    ezipEncContext *c = avctx->priv_data;
    enum AVPixelFormat pix_fmt;
    int width, height;
    int ret, video_dst_bufsize;
    uint8_t *video_dst_data[4] = {NULL};
    int video_dst_linesize[4];
    int max_size = avctx->width * avctx->height * 3;

    if ((ret = ff_alloc_packet(avctx, pkt, max_size)) < 0)
        return ret;
    width = p->width;
    height = p->height;
    pix_fmt = p->format;
    ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             width, height, pix_fmt, 1);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR,"encode no memory\n");
        exit(ret);
        return -1;
    }
    video_dst_bufsize = ret;
    av_image_copy(video_dst_data, video_dst_linesize,
                  (const uint8_t **)(p->data), p->linesize,
                  pix_fmt, width, height);


    if(avctx->pix_fmt != AV_PIX_FMT_YUV420P){
        av_log(avctx, AV_LOG_ERROR, "Format not supported!\n");
        return -1;
    }
    char filename[1024];
    char filename_ext[1024];
    snprintf(filename, sizeof(filename) - 1, ".\\eziptemp\\%d_%d_%d", p->coded_picture_number, width, height);
    strcpy(filename_ext, filename);
    strcat(filename_ext, ".yuv");
    FILE *video_dst_file = fopen(filename_ext, "wb+");
    fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
    fclose(video_dst_file);
    char cmd[1024];
    const char* color;
    if (c->is_565)
    {
        color = "-rgb565";
    }
    else
    {
        color = "-rgb888";
    }
#if 1
    snprintf(cmd, sizeof(cmd), ".\\ffmpeg6.exe -pix_fmt yuv420p -s %dx%d -i %s.yuv -update 1 %s.png", width, height, filename, filename);
    system(cmd);
    snprintf(cmd, sizeof(cmd), ".\\eZIP.exe -convert %s.png -binfile 2 -dither 0 -binext .ezip %s -outdir eziptemp", filename, color);
    system(cmd);
#else
    snprintf(cmd, sizeof(cmd), "-pix_fmt yuv420p -s %dx%d -i %s.yuv -update 1 %s", width, height, filename, filename);
    ShellExecuteA(NULL, "open", ".\\ffmpeg.exe", cmd, NULL, SW_HIDE);
    snprintf(cmd, sizeof(cmd), "-convert %s.png -binfile 2 -binext .ezip %s -outdir eziptemp", filename, color);
    ShellExecuteA(NULL, "open", ".\\eZIP.exe", cmd, NULL, SW_HIDE);
#endif
    snprintf(cmd, sizeof(cmd), "%s.ezip", filename);
    FILE *fp = fopen(cmd, "rb");
    if (!fp)
    {
        av_log(NULL, AV_LOG_ERROR, "ezip file error");
        exit(-1);
        av_assert0(0);
    }
    fseek(fp, 0, SEEK_END);
    pkt->size = ftell(fp) - 4;
    fseek(fp, 4, SEEK_SET);
    fread(pkt->data, 1, pkt->size, fp);
    fclose(fp);
    *got_packet = 1;
    av_free(video_dst_data[0]);
    return 0;
}

static av_cold int encode_init_565(AVCodecContext *avctx)
{
    ezipEncContext *c = avctx->priv_data;

    c->avctx= avctx;

    av_assert0(avctx->width && avctx->height);

    c->is_565 = 1;
    return 0;
}

static av_cold int encode_end(AVCodecContext *avctx)
{
    ezipEncContext *c = avctx->priv_data;

    return 0;
}

static av_cold int encode_init_888(AVCodecContext *avctx)
{
    ezipEncContext *c = avctx->priv_data;

    c->avctx= avctx;

    av_assert0(avctx->width && avctx->height);

    avctx->extradata = av_mallocz(8 + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!avctx->extradata)
        return AVERROR(ENOMEM);

    c->compression = avctx->compression_level;
    c->is_565 = 0;
    return 0;
}

const FFCodec ff_ezip565_encoder = {
    .p.name         = "ezip",
    .p.long_name    = NULL_IF_CONFIG_SMALL("sifli ezip 565"),
    .p.type         = AVMEDIA_TYPE_VIDEO,
    .p.id           = AV_CODEC_ID_EZIP565,
    .priv_data_size = sizeof(ezipEncContext),
    .init           = encode_init_565,
    FF_CODEC_ENCODE_CB(encode_frame),
    .close          = encode_end,
    .p.capabilities = AV_CODEC_CAP_FRAME_THREADS,
    .p.pix_fmts     = (const enum AVPixelFormat[]) { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};
const FFCodec ff_ezip888_encoder = {
    .p.name         = "e888",
    .p.long_name    = NULL_IF_CONFIG_SMALL("sifli ezip 565"),
    .p.type         = AVMEDIA_TYPE_VIDEO,
    .p.id           = AV_CODEC_ID_EZIP888,
    .priv_data_size = sizeof(ezipEncContext),
    .init           = encode_init_888,
    FF_CODEC_ENCODE_CB(encode_frame),
    .close          = encode_end,
    .p.capabilities = AV_CODEC_CAP_FRAME_THREADS,
    .p.pix_fmts     = (const enum AVPixelFormat[]) {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE |
                      FF_CODEC_CAP_INIT_CLEANUP,
};

