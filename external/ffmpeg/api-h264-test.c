#include <stdbool.h>
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include <libswscale/swscale.h>
#include "libavutil/samplefmt.h"
#include "libswresample/swresample.h"
#include "yuv2rgb/yuv2rgb.h"
# include <rtthread.h>
#if 1
#if FFMPEG_SW_SCALE
static struct SwsContext* src_sws = NULL;
#endif
extern AVCodec ff_h264_decoder;
extern AVCodecParser ff_h264_parser;
extern AVCodec ff_aac_decoder;
extern AVCodecParser ff_aac_parser;
extern AVCodec ff_aac_fixed_decoder;


static int decode_write_frame(AVCodecContext* avctx, AVFrame* frame, int* frame_index, AVPacket* pkt, int flush)
{
    int got_frame = 0;
    do {
        int len = avcodec_decode_video2(avctx, frame, &got_frame, pkt);
        if (len < 0) {
            //fprintf(stderr, "Error while decoding frame %d\n", *frame_index);
            printf("Error while decoding frame %d\n!", *frame_index);
            return len;
        }

        if (got_frame) {
            printf("%d\r\n", *frame_index);
            static uint8_t* out_buffer = NULL;
            static AVFrame* pFrameYUV = NULL;
            int res;
            if (pFrameYUV == NULL)
            {
                out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB565, frame->width, frame->height, 1));
                pFrameYUV = av_frame_alloc();
                if (!pFrameYUV)
                    printf("error alloc frame\n");

                res = av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_RGB565, frame->width, frame->height, 1);
                if (res < 0) {
                    printf("av_image_fill_linesizes failed\n");
                }
            }
#if !FFMPEG_SW_SCALE
			yuv420_2_rgb565(out_buffer,
							 (const uint8_t* const*)frame->data,
							 (const uint8_t* const*)frame->data + frame->width * frame->height,
							 (const uint8_t* const*)frame->data + frame->width * frame->height + frame->width * frame->height/2,
							 frame->width,
							 frame->height,
							 frame->width,
							 frame->width>>1,
							 frame->width<<1,
							 yuv2rgb565_table,
							 0);
			printf("--%d\n", out_buffer[129]);
#else
            do {

                src_sws = sws_getCachedContext(src_sws, frame->width,
                    frame->height,
                    frame->format, //AV_PIX_FMT_YUVA420P
                    frame->width,
                    frame->height,
                    AV_PIX_FMT_RGB565,
                    SWS_BILINEAR, NULL, NULL, NULL);
                if (!src_sws) {
                    fprintf(stderr, "Failed to get src scale context\n");
                    res = -1;
                    break;
                }
                res = sws_scale(src_sws, (const uint8_t* const*)frame->data, (const int*)frame->linesize, 0, frame->height, pFrameYUV->data, pFrameYUV->linesize);
                if (res < 0 || res != frame->height) {
                    printf("sws_scale failed\n");
                    res = -1;
                    break;
                }
            } while (0);
#endif
            (*frame_index)++;
        }
        else {
            printf("got jimo\n");
        }
    } while (flush && got_frame);

    return 0;
}

const uint8_t h264_data[10];
static void h264_video_decode(void *p)
{
    uint8_t *p_h264;
    int count = sizeof(h264_data);
    avcodec_register(&ff_h264_decoder);
    av_register_codec_parser(&ff_h264_parser);


    AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        printf("Codec not found\n");
        return;
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        printf("Could not allocate video codec context\n");
        return;
    }

    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        printf("Could not open codec\n");
        return;
    }

    AVCodecParserContext* parser = av_parser_init(AV_CODEC_ID_H264);
    if (!parser) {
        printf("Could not create H264 parser\n");
        return;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        printf("Could not allocate video frame\n");
        return;
    }

    int ending = 0;
    int need_more = 0;
    int frame_index = 0;

    int bytes_read;

    unsigned int buf_len = 0;
    AVPacket packet;

    while (!ending)
    {
        uint8_t* data = NULL;
        uint32_t count;
        uint32_t size = 0;
        uint32_t bytes_used;
        uint32_t end;

        if (count > 0)
        {
            bytes_used = av_parser_parse2(parser, codec_ctx, &data, &size, p_h264, end, 0, 0, AV_NOPTS_VALUE);
            count -= bytes_used;
            p_h264 += bytes_used;
            if (size == 0) {
                break;
            }
            // We have data of one packet, decode it; or decode whatever when ending
            av_init_packet(&packet);
            packet.data = data;
            packet.size = size;
            int ret = decode_write_frame(codec_ctx, frame, &frame_index, &packet, 0);
            if (ret < 0) {
                //fprintf(stderr, "Decode or write frame error\n");
                printf("Decode or write frame error\n");
                break;
            }

        }
        else
        {
           break;
        }
    }
    // Flush the decoder
    packet.data = NULL;
    packet.size = 0;
    decode_write_frame(codec_ctx, frame, &frame_index, &packet, 1);
#if FFMPEG_SW_SCALE
    sws_freeContext(src_sws);
#endif
    avcodec_close(codec_ctx);
    av_free(codec_ctx);
    av_parser_close(parser);
    av_frame_free(&frame);
    printf("Done\n");

}
#endif
#if 0
extern const AVCodec ff_mp3float_decoder;
extern const AVCodec ff_mp3_decoder;
extern const AVCodecParser ff_mpegaudio_parser;

#include "mp3.data"
#include "aac.data"
#define DEBUG_SIZE   (1*1024*1024)
extern void *app_anim_mem_alloc(unsigned long size, bool anim_data);
extern void app_anim_mem_free(void *p);




static void audio_test(const char *id)
{
    int g_frames = 0;
    int g_nsamples;
    int g_sample_rate;
    uint8_t *pp = (uint8_t *) mp3_data;
    int file_size = sizeof(mp3_data);
    int16_t *out_p16 = (int16_t *)app_anim_mem_alloc(DEBUG_SIZE, 1);
    uint8_t *out_p16_old = (uint8_t *)out_p16;
    avcodec_register(&ff_mp3_decoder);
    avcodec_register(&ff_mp3float_decoder);
    //avcodec_register(&ff_aac_decoder);
    //avcodec_register(&ff_aac_fixed_decoder);

    av_register_codec_parser(&ff_mpegaudio_parser);
    AVCodec* codec = NULL;
    int core_id = AV_CODEC_ID_AAC;
    if (strstr(id, "mp3float"))
    {
        codec = avcodec_find_decoder_by_name("mp3float");
        core_id = AV_CODEC_ID_MP3;
    }
    else if (strstr(id, "mp3"))
    {
        codec = avcodec_find_decoder_by_name("mp3");
        core_id = AV_CODEC_ID_MP3;
    }
    else if (strstr(id, "aac_fixed"))
    {
        pp = (uint8_t *) aac_data;
        file_size = sizeof(aac_data);
        codec = avcodec_find_decoder_by_name("aac_fixed");
    }
    else if (strstr(id, "aac"))
    {
        pp = (uint8_t *) aac_data;
        file_size = sizeof(aac_data);
        codec = avcodec_find_decoder_by_name("aac");
    }

    if (!codec) {
        printf("Codec not found\n");
        return;
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        printf("Could not allocate video codec context\n");
        return;
    }

    codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;


    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        printf("Could not open codec\n");
        return;
    }

    AVCodecParserContext* parser = av_parser_init(core_id);
    if (!parser) {
        printf("Could not create H264 parser\n");
        return;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        printf("Could not allocate video frame\n");
        return;
    }

    int ending = 0;

    int frame_index = 0;

    uint32_t bytes_read;

    AVPacket packet;
    int got_frame;
 
    enum AVSampleFormat sfmt = codec_ctx->sample_fmt;
    printf("out format=%d]n", sfmt);
    rt_tick_t t0 = rt_tick_get_millisecond();
    while (1)
    {
        uint8_t* data = NULL;
        uint32_t count;
        uint32_t size = 0;
        int bytes_used;
        uint32_t end;
        if (file_size > 0)
        {
            bytes_used = av_parser_parse2(parser, codec_ctx, &data, &size, pp, file_size, 0, 0, AV_NOPTS_VALUE);
            file_size -= bytes_used;
            pp += bytes_used;
            if (size == 0) {
                continue;
            }
            // We have data of one packet, decode it; or decode whatever when ending
            av_init_packet(&packet);
            packet.data = data;
            packet.size = size;
            int ret = avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);
            if (ret < 0)
            {
                printf("Decode frame error\n");
            }
            else
            {
                if (got_frame)
                {
                    g_frames++;
                    g_nsamples = frame->nb_samples;
                    g_sample_rate = frame->sample_rate;
                    
                    if (sfmt == AV_SAMPLE_FMT_S16P && out_p16 && (uint8_t*)out_p16 < (out_p16_old + DEBUG_SIZE - 6000))
                    { // Audacity: 16bit PCM little endian stereo
                        int16_t* ptr_l = (int16_t*)frame->extended_data[0];
                        int16_t* ptr_r = (int16_t*)frame->extended_data[1];
                        for (int i = 0; i < frame->nb_samples; i++)
                        {
                            *out_p16++ = ptr_l++;
                            *out_p16++ = ptr_r++;
                        }
                    }
                    else if (sfmt == AV_SAMPLE_FMT_S16 && out_p16 && (uint8_t*)out_p16 < (out_p16_old + DEBUG_SIZE - 6000))
                    {
                        int16_t* ptr_l = (int16_t*)frame->extended_data[0];
                        memcpy(out_p16, ptr_l, frame->nb_samples * sizeof(int16_t) * frame->channels);
                        out_p16 += frame->nb_samples * frame->channels;

                    }
                    else if (AV_SAMPLE_FMT_FLT == sfmt && out_p16  && (uint8_t*)out_p16 < (out_p16_old + DEBUG_SIZE - 6000))
                    {
                        float * ptr_l = (float*)frame->extended_data[0];
                        int16_t l16, r16;
                        for (int i = 0; i < frame->nb_samples; i++)
                        {
                            float l_f = *ptr_l++;
                            float r_f = *ptr_l++;

                            if (l_f > +0.999999f)
                                l16 = 0x7FFF;
                            else if (l_f < -0.999999f)
                                l16 = 0x8000;
                            else
                                l16 = l_f * 32767.0f;

                            if (r_f > +0.999999f)
                                r16 = 0x7FFF;
                            else if (r_f < -0.999999f)
                                r16 = 0x8000;
                            else
                                r16 = r_f * 32767.0f;

                            *out_p16++ = l16;
                            *out_p16++ = r16;

                        }
                    } else {

                        struct SwrContext * swrContext = swr_alloc();
                        enum AVSampleFormat inFormat = sfmt;
                        enum AVSampleFormat  outFormat = AV_SAMPLE_FMT_S16;

                        int inSampleRate = codec_ctx->sample_rate;

                        int outSampleRate = inSampleRate;

                        uint64_t in_ch_layout = codec_ctx->channel_layout;

                        uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

                        swr_alloc_set_opts(swrContext, out_ch_layout, outFormat, outSampleRate,
                            in_ch_layout, inFormat, inSampleRate, 0, NULL);

                        if (swr_init(swrContext) < 0)
                        {
                            printf("swr_init error\n");
                            break;
                        }

                        int outChannelCount = av_get_channel_layout_nb_channels(out_ch_layout);
                        int src_nb_channels = av_get_channel_layout_nb_channels(in_ch_layout);

                        int currentIndex = 0;
                        printf("channel number%d in %d\n", outChannelCount, src_nb_channels);

                        {
                            uint8_t** out_buffer = NULL;
                            int out_buffer_size = av_samples_get_buffer_size(NULL, outChannelCount, frame->nb_samples, outFormat, 1);
                            av_samples_alloc_array_and_samples(&out_buffer, NULL, outChannelCount,
                                    frame->nb_samples, outFormat, 0);

                            int nCvtedSamples = swr_convert(swrContext, out_buffer, out_buffer_size,
                                            (const uint8_t**)frame->data, frame->nb_samples);

                            printf("convert=%d\n", nCvtedSamples);
                            if (nCvtedSamples == frame->nb_samples && out_p16  && (uint8_t*)out_p16 < (out_p16_old + DEBUG_SIZE - 6000))
                                memcpy(out_p16, out_buffer[0], out_buffer_size);

                            if (out_buffer)
                                av_freep(&out_buffer[0]);
                            av_freep(&out_buffer);
                            swr_free(&swrContext);
                        }
                    }
                }
            }
        }
        else {
            break;
        }

    }
    // Flush the decoder
    packet.data = NULL;
    packet.size = 0;

    int len = avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);

    rt_tick_t t1 = rt_tick_get_millisecond();

    uint32_t fps = (uint32_t)((uint64_t)g_frames * 10000 / (t1 - t0));
    
    rt_kprintf("decode speed=%dKbits, g_samplerate = %d g_out_start=0x%x, g_out=0x%x\n", g_frames * g_nsamples *16 / 1000, g_sample_rate, out_p16_old, out_p16);
    rt_kprintf("frames=%d, fps = %d.%d\n", g_frames, fps/10, fps%10);


    avcodec_close(codec_ctx);
    av_free(codec_ctx);
#if FFMPEG_SW_SCALE
static struct SwsContext* src_sws = NULL;
#endif
    av_parser_close(parser);
    av_frame_free(&frame);
    if (out_p16)
        app_anim_mem_free(out_p16);
    printf("Done\n");

}
#endif
int audiotest(int argc, char *argv[])
{
    rt_thread_t thread;

    if (argc < 2)
    {
        av_log(NULL, AV_LOG_ERROR, "Incorrect input\n");
        return 1;
    }

    if (strstr(argv[1], "h264") || strstr(argv[1], "mp4"))
    {
        thread = rt_thread_create("h264", h264_video_decode, NULL, 16000, 14, 10);
    }
    else if (strstr(argv[1], "mp3"))
    {
        //thread = rt_thread_create("mp3aac", audio_test, (void*)argv[1], 16000, 14, 10);
    }

    rt_kprintf("thread=%d\n", thread);
    rt_thread_mdelay(100);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    return 0;
}

MSH_CMD_EXPORT(audiotest, audio test);




