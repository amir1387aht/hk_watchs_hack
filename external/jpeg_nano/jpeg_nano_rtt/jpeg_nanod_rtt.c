/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "string.h"

#include "bf0_hal.h"
#include "drv_common.h"
#include "drv_io.h"
#include "drv_lcd.h"

#include "jpegdecapi.h"
#include "dwl.h"
#include "jpegdecapi_ex.h"

#ifdef PP_PIPELINE_ENABLED
    #include "ppapi.h"
#endif

#ifdef ASIC_TRACE_SUPPORT
    #include "trace.h"
#endif

#include "deccfg.h"

#ifndef ABS
    #define ABS(_x) ((_x) < 0 ? -(_x) : (_x))
#endif

/**********************
 *      MACROS
 **********************/

#define TC_JPEG_NANOD_LOG_I LOG_I
#define TC_JPEG_NANOD_LOG_E LOG_E
#define TC_JPEG_PP_LOG_I LOG_I
#define  DBG_LEVEL            DBG_ERROR  //DBG_INFO  //DBG_LOG //
#undef LOG_TAG
#define LOG_TAG                "TC.JPEG.NANOD"
#include "log.h"

#define OUTPUT_TO_LCD

#if 1//def SW_PERFORMANCE
#define INIT_SW_PERFORMANCE\
    clock_t dec_cpu_time = 0;\
    clock_t dec_start_time = 0;\
    clock_t dec_end_time = 0; \
    clock_t dec_g_start = rt_tick_get();

#define START_SW_PERFORMANCE\
        dec_start_time = rt_tick_get();

#define END_SW_PERFORMANCE\
        dec_end_time = rt_tick_get();\
        dec_cpu_time += dec_end_time - dec_start_time;

#define FINALIZE_SW_PERFORMANCE\
            LOG_I("SW_PERFORMANCE %d, total=%d, CLOCKS_PER_SEC=%d\n", dec_cpu_time, rt_tick_get()- dec_g_start, RT_TICK_PER_SECOND);

typedef enum
{
    ARGI_JPEGD_IN_FILE_NAME,
    ARGI_JPEGD_OUT_FORMAT,
    ARGI_MAX,

} ARG_IDX;

#else
#define INIT_SW_PERFORMANCE
#define START_SW_PERFORMANCE
#define END_SW_PERFORMANCE
#define FINALIZE_SW_PERFORMANCE

#endif

static JpegDecInst jpeg;
static JpegDecImageInfo imageInfo;
static JpegDecInput jpegIn;
static JpegDecOutput jpegOut;

/* user allocated output */
static DWLLinearMem_t userAllocLuma;
static DWLLinearMem_t userAllocChroma;
static DWLLinearMem_t userAllocCr;

static u32 prevOutputWidth = 0;
static u32 prevOutputHeight = 0;
static u32 prevOutputFormat = 0;
static u32 prevOutputWidthTn = 0;
static u32 prevOutputHeightTn = 0;
static u32 prevOutputFormatTn = 0;

static rt_device_t tc_lcd_device = NULL;
static uint32_t lcd_width, lcd_height, lcd_align;

static u32 is8170HW = 0;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
static u32 GetLumaSize(JpegDecImageInfo *imageInfo, u32 picMode)
{
    u32 format;
    u32 sizeLuma = 0;

    format = picMode == 0 ?
             imageInfo->outputFormat : imageInfo->outputFormatThumb;

    if (picMode == 0)   /* full */
    {
        sizeLuma = (imageInfo->outputWidth * imageInfo->outputHeight);
    }
    else    /* thumbnail */
    {
        sizeLuma =
            (imageInfo->outputWidthThumb * imageInfo->outputHeightThumb);
    }

    return sizeLuma;
}

static u32 GetChromaSize(JpegDecImageInfo *imageInfo, u32 picMode)
{
    u32 sizeChroma, format;
    u32 sizeLuma = GetLumaSize(imageInfo, picMode);

    format = picMode == 0 ?
             imageInfo->outputFormat : imageInfo->outputFormatThumb;

    if (format != JPEGDEC_YCbCr400)
    {
        if (format == JPEGDEC_YCbCr420_SEMIPLANAR ||
                format == JPEGDEC_YCbCr411_SEMIPLANAR)
        {
            sizeChroma = (sizeLuma / 2);
        }
        else if (format == JPEGDEC_YCbCr444_SEMIPLANAR)
        {
            sizeChroma = sizeLuma * 2;
        }
        else
        {
            sizeChroma = sizeLuma;
        }
    }

    return sizeChroma;
}

/*-----------------------------------------------------------------------------

Print JpegDecGetImageInfo values

-----------------------------------------------------------------------------*/
static void PrintGetImageInfo(JpegDecImageInfo *imageInfo)
{
    RT_ASSERT(imageInfo);

    /* Select if Thumbnail or full resolution image will be decoded */
    if (imageInfo->thumbnailType == JPEGDEC_THUMBNAIL_JPEG)
    {
        /* decode thumbnail */
        TC_JPEG_NANOD_LOG_I("\t-JPEG THUMBNAIL IN STREAM\n");
        TC_JPEG_NANOD_LOG_I("\t-JPEG THUMBNAIL INFO\n");
        TC_JPEG_NANOD_LOG_I("\t\t-JPEG thumbnail width: %d\n",
                            imageInfo->outputWidthThumb);
        TC_JPEG_NANOD_LOG_I("\t\t-JPEG thumbnail height: %d\n",
                            imageInfo->outputHeightThumb);

        /* stream type */
        switch (imageInfo->codingModeThumb)
        {
        case JPEGDEC_BASELINE:
            TC_JPEG_NANOD_LOG_I("\t\t-JPEG: STREAM TYPE: JPEGDEC_BASELINE\n");
            break;
        case JPEGDEC_PROGRESSIVE:
            TC_JPEG_NANOD_LOG_I("\t\t-JPEG: STREAM TYPE: JPEGDEC_PROGRESSIVE\n");
            break;
        case JPEGDEC_NONINTERLEAVED:
            TC_JPEG_NANOD_LOG_I("\t\t-JPEG: STREAM TYPE: JPEGDEC_NONINTERLEAVED\n");
            break;
        }

        if (imageInfo->outputFormatThumb)
        {
            switch (imageInfo->outputFormatThumb)
            {
            case JPEGDEC_YCbCr400:
                TC_JPEG_NANOD_LOG_I(
                    "\t\t-JPEG: THUMBNAIL OUTPUT: JPEGDEC_YCbCr400\n");
                break;
            case JPEGDEC_YCbCr420_SEMIPLANAR:
                TC_JPEG_NANOD_LOG_I(
                    "\t\t-JPEG: THUMBNAIL OUTPUT: JPEGDEC_YCbCr420_SEMIPLANAR\n");
                break;
            case JPEGDEC_YCbCr422_SEMIPLANAR:
                TC_JPEG_NANOD_LOG_I(
                    "\t\t-JPEG: THUMBNAIL OUTPUT: JPEGDEC_YCbCr422_SEMIPLANAR\n");
                break;
            case JPEGDEC_YCbCr440:
                TC_JPEG_NANOD_LOG_I(
                    "\t\t-JPEG: THUMBNAIL OUTPUT: JPEGDEC_YCbCr440\n");
                break;
            case JPEGDEC_YCbCr411_SEMIPLANAR:
                TC_JPEG_NANOD_LOG_I(
                    "\t\t-JPEG: THUMBNAIL OUTPUT: JPEGDEC_YCbCr411_SEMIPLANAR\n");
                break;
            case JPEGDEC_YCbCr444_SEMIPLANAR:
                TC_JPEG_NANOD_LOG_I(
                    "\t\t-JPEG: THUMBNAIL OUTPUT: JPEGDEC_YCbCr444_SEMIPLANAR\n");
                break;
            }
        }
    }
    else if (imageInfo->thumbnailType == JPEGDEC_NO_THUMBNAIL)
    {
        /* decode full image */
        TC_JPEG_NANOD_LOG_I(
            "\t-NO THUMBNAIL IN STREAM ==> Decode full resolution image\n");
    }
    else if (imageInfo->thumbnailType == JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT)
    {
        /* decode full image */
        TC_JPEG_NANOD_LOG_I(
            "\tNOT SUPPORTED THUMBNAIL IN STREAM ==> Decode full resolution image\n");
    }

    TC_JPEG_NANOD_LOG_I("\t-JPEG FULL RESOLUTION INFO\n");
    TC_JPEG_NANOD_LOG_I("\t\t-JPEG width: %d\n", imageInfo->outputWidth);
    TC_JPEG_NANOD_LOG_I("\t\t-JPEG height: %d\n", imageInfo->outputHeight);
    if (imageInfo->outputFormat)
    {
        switch (imageInfo->outputFormat)
        {
        case JPEGDEC_YCbCr400:
            TC_JPEG_NANOD_LOG_I(
                "\t\t-JPEG: FULL RESOLUTION OUTPUT: JPEGDEC_YCbCr400\n");
#ifdef ASIC_TRACE_SUPPORT
            trace_jpegDecodingTools.sampling_4_0_0 = 1;
#endif
            break;
        case JPEGDEC_YCbCr420_SEMIPLANAR:
            TC_JPEG_NANOD_LOG_I(
                "\t\t-JPEG: FULL RESOLUTION OUTPUT: JPEGDEC_YCbCr420_SEMIPLANAR\n");
#ifdef ASIC_TRACE_SUPPORT
            trace_jpegDecodingTools.sampling_4_2_0 = 1;
#endif
            break;
        case JPEGDEC_YCbCr422_SEMIPLANAR:
            TC_JPEG_NANOD_LOG_I(
                "\t\t-JPEG: FULL RESOLUTION OUTPUT: JPEGDEC_YCbCr422_SEMIPLANAR\n");
#ifdef ASIC_TRACE_SUPPORT
            trace_jpegDecodingTools.sampling_4_2_2 = 1;
#endif
            break;
        case JPEGDEC_YCbCr440:
            TC_JPEG_NANOD_LOG_I(
                "\t\t-JPEG: FULL RESOLUTION OUTPUT: JPEGDEC_YCbCr440\n");
#ifdef ASIC_TRACE_SUPPORT
            trace_jpegDecodingTools.sampling_4_4_0 = 1;
#endif
            break;
        case JPEGDEC_YCbCr411_SEMIPLANAR:
            TC_JPEG_NANOD_LOG_I(
                "\t\t-JPEG: FULL RESOLUTION OUTPUT: JPEGDEC_YCbCr411_SEMIPLANAR\n");
#ifdef ASIC_TRACE_SUPPORT
            trace_jpegDecodingTools.sampling_4_1_1 = 1;
#endif
            break;
        case JPEGDEC_YCbCr444_SEMIPLANAR:
            TC_JPEG_NANOD_LOG_I(
                "\t\t-JPEG: FULL RESOLUTION OUTPUT: JPEGDEC_YCbCr444_SEMIPLANAR\n");
#ifdef ASIC_TRACE_SUPPORT
            trace_jpegDecodingTools.sampling_4_4_4 = 1;
#endif
            break;
        }
    }

    /* stream type */
    switch (imageInfo->codingMode)
    {
    case JPEGDEC_BASELINE:
        TC_JPEG_NANOD_LOG_I("\t\t-JPEG: STREAM TYPE: JPEGDEC_BASELINE\n");
        break;
    case JPEGDEC_PROGRESSIVE:
        TC_JPEG_NANOD_LOG_I("\t\t-JPEG: STREAM TYPE: JPEGDEC_PROGRESSIVE\n");
#ifdef ASIC_TRACE_SUPPORT
        trace_jpegDecodingTools.progressive = 1;
#endif
        break;
    case JPEGDEC_NONINTERLEAVED:
        TC_JPEG_NANOD_LOG_I("\t\t-JPEG: STREAM TYPE: JPEGDEC_NONINTERLEAVED\n");
        break;
    }

    if (imageInfo->thumbnailType == JPEGDEC_THUMBNAIL_JPEG)
    {
        TC_JPEG_NANOD_LOG_I("\t-JPEG ThumbnailType: JPEG\n");
#ifdef ASIC_TRACE_SUPPORT
        trace_jpegDecodingTools.thumbnail = 1;
#endif
    }
    else if (imageInfo->thumbnailType == JPEGDEC_NO_THUMBNAIL)
        TC_JPEG_NANOD_LOG_I("\t-JPEG ThumbnailType: NO THUMBNAIL\n");
    else if (imageInfo->thumbnailType == JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT)
        TC_JPEG_NANOD_LOG_I("\t-JPEG ThumbnailType: NOT SUPPORTED THUMBNAIL\n");
}

/*-----------------------------------------------------------------------------

Print JPEG api return value

-----------------------------------------------------------------------------*/
static void PrintJpegRet(JpegDecRet *pJpegRet)
{

    RT_ASSERT(pJpegRet);

    switch (*pJpegRet)
    {
    case JPEGDEC_FRAME_READY:
        TC_JPEG_NANOD_LOG_I("TB: jpeg API returned : JPEGDEC_FRAME_READY\n");
        break;
    case JPEGDEC_OK:
        TC_JPEG_NANOD_LOG_I("TB: jpeg API returned : JPEGDEC_OK\n");
        break;
    case JPEGDEC_ERROR:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_ERROR\n");
        break;
    case JPEGDEC_DWL_HW_TIMEOUT:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_HW_TIMEOUT\n");
        break;
    case JPEGDEC_UNSUPPORTED:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_UNSUPPORTED\n");
        break;
    case JPEGDEC_PARAM_ERROR:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_PARAM_ERROR\n");
        break;
    case JPEGDEC_MEMFAIL:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_MEMFAIL\n");
        break;
    case JPEGDEC_INITFAIL:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_INITFAIL\n");
        break;
    case JPEGDEC_HW_BUS_ERROR:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_HW_BUS_ERROR\n");
        break;
    case JPEGDEC_SYSTEM_ERROR:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_SYSTEM_ERROR\n");
        break;
    case JPEGDEC_DWL_ERROR:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_DWL_ERROR\n");
        break;
    case JPEGDEC_INVALID_STREAM_LENGTH:
        TC_JPEG_NANOD_LOG_E(
            "TB: jpeg API returned : JPEGDEC_INVALID_STREAM_LENGTH\n");
        break;
    case JPEGDEC_STRM_ERROR:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned : JPEGDEC_STRM_ERROR\n");
        break;
    case JPEGDEC_INVALID_INPUT_BUFFER_SIZE:
        TC_JPEG_NANOD_LOG_E(
            "TB: jpeg API returned : JPEGDEC_INVALID_INPUT_BUFFER_SIZE\n");
        break;
    case JPEGDEC_INCREASE_INPUT_BUFFER:
        TC_JPEG_NANOD_LOG_E(
            "TB: jpeg API returned : JPEGDEC_INCREASE_INPUT_BUFFER\n");
        break;
    case JPEGDEC_SLICE_MODE_UNSUPPORTED:
        TC_JPEG_NANOD_LOG_E(
            "TB: jpeg API returned : JPEGDEC_SLICE_MODE_UNSUPPORTED\n");
        break;
    default:
        TC_JPEG_NANOD_LOG_E("TB: jpeg API returned unknown status\n");
        break;
    }
}

/*-----------------------------------------------------------------------------

    Function name:  FindImageInfoEnd

    Purpose:
        Finds 0xFFC4 from the stream and pOffset includes number of bytes to
        this marker. In case of an error returns != 0
        (i.e., the marker not found).

-----------------------------------------------------------------------------*/
static u32 FindImageInfoEnd(u8 *pStream, u32 streamLength, u32 *pOffset)
{
    u32 i;

    for (i = 0; i < streamLength; ++i)
    {
        if (0xFF == pStream[i])
        {
            if (((i + 1) < streamLength) && 0xC4 == pStream[i + 1])
            {
                *pOffset = i;
                return 0;
            }
        }
    }
    return -1;
}

#ifdef PP_PIPELINE_ENABLED

static PPInst ppInst = NULL;
static PPConfig ppConf;

/*------------------------------------------------------------------------------
    Function name   : pp_set_input_interlaced
    Description     : decoder sets up interlaced input type
    Return type     : void
    Argument        : is interlaced or not
------------------------------------------------------------------------------*/
static const char *pp_result2str(PPResult ret)
{
    const char *str;

    switch (ret)
    {
    case PP_OK:
        str = "PP_OK";
        break;
    case PP_PARAM_ERROR:
        str = "PP_PARAM_ERROR";
        break;
    case PP_MEMFAIL:
        str = "PP_MEMFAIL";
        break;
    case PP_SET_IN_SIZE_INVALID:
        str = "PP_SET_IN_SIZE_INVALID";
        break;
    case PP_SET_IN_ADDRESS_INVALID:
        str = "PP_SET_IN_ADDRESS_INVALID";
        break;
    case PP_SET_IN_FORMAT_INVALID:
        str = "PP_SET_IN_FORMAT_INVALID";
        break;
    case PP_SET_CROP_INVALID:
        str = "PP_SET_CROP_INVALID";
        break;
    case PP_SET_ROTATION_INVALID:
        str = "PP_SET_ROTATION_INVALID";
        break;
    case PP_SET_OUT_SIZE_INVALID:
        str = "PP_SET_OUT_SIZE_INVALID";
        break;
    case PP_SET_OUT_ADDRESS_INVALID:
        str = "PP_SET_OUT_ADDRESS_INVALID";
        break;
    case PP_SET_OUT_FORMAT_INVALID:
        str = "PP_SET_OUT_FORMAT_INVALID";
        break;
    case PP_SET_VIDEO_ADJUST_INVALID:
        str = "PP_SET_VIDEO_ADJUST_INVALID";
        break;
    case PP_SET_RGB_BITMASK_INVALID:
        str = "PP_SET_RGB_BITMASK_INVALID";
        break;
    case PP_SET_FRAMEBUFFER_INVALID:
        str = "PP_SET_FRAMEBUFFER_INVALID";
        break;
    case PP_SET_MASK1_INVALID:
        str = "PP_SET_MASK1_INVALID";
        break;
    case PP_SET_MASK2_INVALID:
        str = "PP_SET_MASK2_INVALID";
        break;
    case PP_SET_DEINTERLACE_INVALID:
        str = "PP_SET_DEINTERLACE_INVALID";
        break;
    case PP_SET_IN_STRUCT_INVALID:
        str = "PP_SET_IN_STRUCT_INVALID";
        break;
    case PP_SET_IN_RANGE_MAP_INVALID:
        str = "PP_SET_IN_RANGE_MAP_INVALID";
        break;
    case PP_SET_ABLEND_UNSUPPORTED:
        str = "PP_SET_ABLEND_UNSUPPORTED";
        break;
    case PP_SET_DEINTERLACING_UNSUPPORTED:
        str = "PP_SET_DEINTERLACING_UNSUPPORTED";
        break;
    case PP_SET_DITHERING_UNSUPPORTED:
        str = "PP_SET_DITHERING_UNSUPPORTED";
        break;
    case PP_SET_SCALING_UNSUPPORTED:
        str = "PP_SET_SCALING_UNSUPPORTED";
        break;
    case PP_BUSY:
        str = "PP_BUSY";
        break;
    case PP_HW_BUS_ERROR:
        str = "PP_HW_BUS_ERROR";
        break;
    case PP_HW_TIMEOUT:
        str = "PP_HW_TIMEOUT";
        break;
    case PP_DWL_ERROR:
        str = "PP_DWL_ERROR";
        break;
    case PP_SYSTEM_ERROR:
        str = "PP_SYSTEM_ERROR";
        break;
    case PP_DEC_COMBINED_MODE_ERROR:
        str = "PP_DEC_COMBINED_MODE_ERROR";
        break;
    case PP_DEC_RUNTIME_ERROR:
        str = "PP_DEC_RUNTIME_ERROR";
        break;
    default:
        str = "Unknow";
        break;
    }

    return str;
}

/*------------------------------------------------------------------------------
    Function name   : pp_api_release
    Description     : Release the post processor. Disable pipeline if needed.
    Return type     : void
    Argument        : PPInst * pp
------------------------------------------------------------------------------*/
static void pp_api_release(void *decoder)
{
    PPOutImage *ppOutImg = &ppConf.ppOutImg;
    /*
        u8 *pb;

        pb = (u8 *)ppOutImg->bufferBusAddr;

        TC_JPEG_PP_LOG_I("pp_api_release buf=%x, +0x268C ~ 0x268F = %x, %x, %x, %x",
            pb, *(pb + 0x268C), *(pb + 0x268D),*(pb + 0x268E),*(pb + 0x268F));
    */
    DWLfree((void *)ppOutImg->bufferBusAddr);

    if (ppInst != NULL && decoder != NULL)
        PPDecCombinedModeDisable(ppInst, decoder);

    if (ppInst != NULL)
        PPRelease(ppInst);
}

static void pp_api_release2(void *decoder)
{
    PPOutImage *ppOutImg = &ppConf.ppOutImg;
    /*
        u8 *pb;

        pb = (u8 *)ppOutImg->bufferBusAddr;

        TC_JPEG_PP_LOG_I("pp_api_release buf=%x, +0x268C ~ 0x268F = %x, %x, %x, %x",
            pb, *(pb + 0x268C), *(pb + 0x268D),*(pb + 0x268E),*(pb + 0x268F));
    */
    //DWLfree((void *)ppOutImg->bufferBusAddr);

    if (ppInst != NULL && decoder != NULL)
        PPDecCombinedModeDisable(ppInst, decoder);

    if (ppInst != NULL)
        PPRelease(ppInst);
}

static void pp_api_wait()
{
    PPResult res;

    res = PPGetResult(ppInst);
    TC_JPEG_PP_LOG_I("pp_api_wait %s", pp_result2str(res));

}

static u32 pp_startup(const void *decInst, u32 decType, const JpegDecImageInfo *ImgInfo, u32 out_cf)
{

    PPResult res;

    res = PPInit(&ppInst);

    if (res != PP_OK)
    {
        TC_JPEG_PP_LOG_I("Failed to init the PP: %s\n", pp_result2str(res));

        ppInst = NULL;

        return 1;
    }

    if (decInst != NULL && decType != PP_PIPELINE_DISABLED)
    {
        res = PPDecCombinedModeEnable(ppInst, decInst, decType);

        if (res != PP_OK)
        {
            TC_JPEG_PP_LOG_I("Failed to enable PP-DEC pipeline: %s\n", pp_result2str(res));

            ppInst = NULL;

            return 1;
        }

    }

    // AllocFrame(&pp.currPp->output);

    TC_JPEG_PP_LOG_I("---config PP---\n");

    {
        int ret = 0;
        PPResult res;

        PPOutImage *ppOutImg = &ppConf.ppOutImg;
        PPInImage *ppInImg = &ppConf.ppInImg;
        PPOutRgb *ppOutRgb = &ppConf.ppOutRgb;
        PPInCropping *ppInCrop = &ppConf.ppInCrop;

        u32 buffer_len;

        res = PPGetConfig(ppInst, &ppConf);

        ppInImg->width = ImgInfo->outputWidth;
        ppInImg->height = ImgInfo->outputHeight;

#if 0  //In crop
        ppInCrop->enable  = 1;
        ppInCrop->originX = 16   *         1  ;
        ppInCrop->width   = 8 * (6 +         2);

        ppInCrop->originY = 16   *         2  ;
        ppInCrop->height  = 8 * (2 +         3);

        ppOutImg->width = ppInCrop->width;
        ppOutImg->height = ppInCrop->height;
#elif 0 //Out crop

        PPOutFrameBuffer *ppOutFrmBuffer = &ppConf.ppOutFrmBuffer;

        ppOutFrmBuffer->enable = 1;
        ppOutFrmBuffer->writeOriginX = 14;
        ppOutFrmBuffer->writeOriginY = 12;
        ppOutFrmBuffer->frameBufferWidth = 134;
        ppOutFrmBuffer->frameBufferHeight = 158;

        ppOutImg->width = ImgInfo->outputWidth;
        ppOutImg->height = ImgInfo->outputHeight;

#else
        ppOutImg->width = ImgInfo->outputWidth;
        ppOutImg->height = ImgInfo->outputHeight;

#endif /* 0 */

        buffer_len = ppOutImg->width * ppOutImg->height * 4;
        ppOutImg->bufferBusAddr = (g1_addr_t) DWLmalloc(buffer_len);
        DWLmemset((void *) ppOutImg->bufferBusAddr, 128, buffer_len);
        TC_JPEG_NANOD_LOG_I("pp outimg buffer %x", ppOutImg->bufferBusAddr);

#if 1
        ppOutImg->pixFormat = out_cf;//PP_PIX_FMT_RGB16_5_6_5; //PP_PIX_FMT_RGB32;

        ppOutRgb->alpha = 255 /*params->outRgbFmt.mask[3] */ ;
        ppOutRgb->ditheringEnable = 0;
#else
        if (0)
        {
            ppOutImg->pixFormat = PP_PIX_FMT_RGB16_CUSTOM;
            ppOutRgb->rgbBitmask.maskR = 0xF8;
            ppOutRgb->rgbBitmask.maskG = 0xFC;
            ppOutRgb->rgbBitmask.maskB = 0xF8;
            ppOutRgb->rgbBitmask.maskAlpha = 0xFF;
        }
        else
        {
            //SiFli ARGB8888 format
            ppOutImg->pixFormat = PP_PIX_FMT_RGB32_CUSTOM;

            ppOutRgb->rgbBitmask.maskR = 0xFF00;
            ppOutRgb->rgbBitmask.maskG = 0xFF0000;
            ppOutRgb->rgbBitmask.maskB = 0xFF000000;
            ppOutRgb->rgbBitmask.maskAlpha = 0xFF;

        }

        ppOutRgb->ditheringEnable = 0;
#endif /* 0 */

        res = PPSetConfig(ppInst, &ppConf);

        /* Restore the PP setup */
        /*    memcpy(&ppConf, &tmpConfig, sizeof(PPConfig));
        */
        if (res != PP_OK)
        {
            TC_JPEG_PP_LOG_I("Failed to setup the PP [%s]\n", pp_result2str(res));
            ret = 1;
        }

        /* write deinterlacing parameters */
#if 0
        if (params->deint.enable)
        {
            SetPpRegister(regBase, HWIF_DEINT_BLEND_E, params->deint.blendEnable);
            SetPpRegister(regBase, HWIF_DEINT_THRESHOLD, params->deint.threshold);
            SetPpRegister(regBase, HWIF_DEINT_EDGE_DET,
                          params->deint.edgeDetectValue);
        }
#endif /* 0 */

        return ret;
    }
}

static u32 pp_startup2(const void *decInst, u32 decType, const JpegDecImageInfo *ImgInfo, u32 out_cf, 
            u8 *out_buf,
            u32 out_buf_len)
{

    PPResult res;

    res = PPInit(&ppInst);

    if (res != PP_OK)
    {
        TC_JPEG_PP_LOG_I("Failed to init the PP: %s\n", pp_result2str(res));

        ppInst = NULL;

        return 1;
    }

    if (decInst != NULL && decType != PP_PIPELINE_DISABLED)
    {
        res = PPDecCombinedModeEnable(ppInst, decInst, decType);

        if (res != PP_OK)
        {
            TC_JPEG_PP_LOG_I("Failed to enable PP-DEC pipeline: %s\n", pp_result2str(res));

            ppInst = NULL;

            return 1;
        }

    }

    // AllocFrame(&pp.currPp->output);

    TC_JPEG_PP_LOG_I("---config PP---\n");

    {
        int ret = 0;
        PPResult res;

        PPOutImage *ppOutImg = &ppConf.ppOutImg;
        PPInImage *ppInImg = &ppConf.ppInImg;
        PPOutRgb *ppOutRgb = &ppConf.ppOutRgb;
        PPInCropping *ppInCrop = &ppConf.ppInCrop;

        u32 buffer_len;

        res = PPGetConfig(ppInst, &ppConf);

        ppInImg->width = ImgInfo->outputWidth;
        ppInImg->height = ImgInfo->outputHeight;

#if 0  //In crop
        ppInCrop->enable  = 1;
        ppInCrop->originX = 16   *         1  ;
        ppInCrop->width   = 8 * (6 +         2);

        ppInCrop->originY = 16   *         2  ;
        ppInCrop->height  = 8 * (2 +         3);

        ppOutImg->width = ppInCrop->width;
        ppOutImg->height = ppInCrop->height;
#elif 0 //Out crop

        PPOutFrameBuffer *ppOutFrmBuffer = &ppConf.ppOutFrmBuffer;

        ppOutFrmBuffer->enable = 1;
        ppOutFrmBuffer->writeOriginX = 14;
        ppOutFrmBuffer->writeOriginY = 12;
        ppOutFrmBuffer->frameBufferWidth = 134;
        ppOutFrmBuffer->frameBufferHeight = 158;

        ppOutImg->width = ImgInfo->outputWidth;
        ppOutImg->height = ImgInfo->outputHeight;

#else
        ppOutImg->width = ImgInfo->outputWidth;
        ppOutImg->height = ImgInfo->outputHeight;

#endif /* 0 */

        u32 pixel_bytes;
        
        switch(out_cf)
        {
            case PP_PIX_FMT_RGB16_5_6_5:  pixel_bytes = 2; break;
            case PP_PIX_FMT_BGR16_5_6_5:  pixel_bytes = 2; break;
            default: pixel_bytes = 4;break;
        }
        
        buffer_len = ppOutImg->width * ppOutImg->height * pixel_bytes;

        if(out_buf_len < buffer_len)
        {
            TC_JPEG_NANOD_LOG_E("out_cf=%x, out_buf_len %d < buffer_len %d", out_cf, out_buf_len , buffer_len);
            return 1;
        }
        ppOutImg->bufferBusAddr = (g1_addr_t) out_buf;
        //DWLmemset((void *) ppOutImg->bufferBusAddr, 128, buffer_len);
        TC_JPEG_NANOD_LOG_I("pp outimg buffer %x", ppOutImg->bufferBusAddr);

#if 1
        ppOutImg->pixFormat = out_cf;//PP_PIX_FMT_RGB16_5_6_5; //PP_PIX_FMT_RGB32;

        ppOutRgb->alpha = 255 /*params->outRgbFmt.mask[3] */ ;
        ppOutRgb->ditheringEnable = 0;
#else
        if (0)
        {
            ppOutImg->pixFormat = PP_PIX_FMT_RGB16_CUSTOM;
            ppOutRgb->rgbBitmask.maskR = 0xF8;
            ppOutRgb->rgbBitmask.maskG = 0xFC;
            ppOutRgb->rgbBitmask.maskB = 0xF8;
            ppOutRgb->rgbBitmask.maskAlpha = 0xFF;
        }
        else
        {
            //SiFli ARGB8888 format
            ppOutImg->pixFormat = PP_PIX_FMT_RGB32_CUSTOM;

            ppOutRgb->rgbBitmask.maskR = 0xFF00;
            ppOutRgb->rgbBitmask.maskG = 0xFF0000;
            ppOutRgb->rgbBitmask.maskB = 0xFF000000;
            ppOutRgb->rgbBitmask.maskAlpha = 0xFF;

        }

        ppOutRgb->ditheringEnable = 0;
#endif /* 0 */

        res = PPSetConfig(ppInst, &ppConf);

        /* Restore the PP setup */
        /*    memcpy(&ppConf, &tmpConfig, sizeof(PPConfig));
        */
        if (res != PP_OK)
        {
            TC_JPEG_PP_LOG_I("Failed to setup the PP [%s]\n", pp_result2str(res));
            ret = 1;
        }

        /* write deinterlacing parameters */
#if 0
        if (params->deint.enable)
        {
            SetPpRegister(regBase, HWIF_DEINT_BLEND_E, params->deint.blendEnable);
            SetPpRegister(regBase, HWIF_DEINT_THRESHOLD, params->deint.threshold);
            SetPpRegister(regBase, HWIF_DEINT_EDGE_DET,
                          params->deint.edgeDetectValue);
        }
#endif /* 0 */

        return ret;
    }
}

#endif /* PP_PIPELINE_ENABLED */

static uint32_t str2Jpegcf(const char *str)
{
    if (rt_strncmp(str, "RGB565", 20) == 0)
        return PP_PIX_FMT_RGB16_5_6_5;
    else if (rt_strncmp(str, "ARGB8888", 20) == 0)
        return PP_PIX_FMT_RGB32;
    else
    {
        LOG_E("Unknow format %s",str);
        RT_ASSERT(0);
        return  UINT32_MAX;
    }
}

#if 1

int jpeg_nanod_decode2(uint8_t *byteStrmStart, 
            uint32_t streamTotalLen, 
            const char *out_cf,
            uint8_t *out_buf,
            uint32_t out_buf_len)
{
    int return_status = 1;
    u32 mcuSizeDivider = 0;

    i32 i, j = 0;
    u32 tmp = 0;
    u32 size = 0;
    u32 loop;
    u32 frameCounter = 0;
    u32 inputReadType = 0;
    i32 bufferSize = 0;
    u32 amountOfMCUs = 0;
    u32 mcuInRow = 0;

    JpegDecRet jpegRet;
    JpegDecApiVersion decVer;
    JpegDecBuild decBuild;

#ifdef PP_PIPELINE_ENABLED
    PPApiVersion ppVer;
    PPBuild ppBuild;
#endif

    u32 streamHeaderCorrupt = 0;
    u32 seedRnd = 0;
    u32 streamBitSwap = 0;
    u32 streamTruncate = 0;

    u32 imageInfoLength = 0;
    u32 prevRet = JPEGDEC_STRM_ERROR;

    u32 writeOutput = 0;

    u32 onlyFullResolution = 0;
    static u32 mode = 0; //0 - thumbnail  1-full solution
    u32 ThumbDone = 0;
    u32 outpu_cf = str2Jpegcf(out_cf);

    /* allocate memory for stream buffer. if unsuccessful -> exit */
    INIT_SW_PERFORMANCE;

    TC_JPEG_NANOD_LOG_I("\n* * * * * * * * * * * * * * * * \n\n\n"
                        "      "
                        "X170 JPEG TESTBENCH\n" "\n\n* * * * * * * * * * * * * * * * \n");

    mpu_dcache_clean((void *) byteStrmStart, (int32_t) streamTotalLen);
#if 0//def OUTPUT_TO_LCD

    if (!tc_lcd_device)
    {
        struct rt_device_graphic_info info;
        uint16_t cf;

        tc_lcd_device = rt_device_find("lcd");

        if (!tc_lcd_device)
        {
            LOG_I("Fail to open lcd");
        }
        else
        {
            /* LCD Device Init */
            rt_device_open(tc_lcd_device, RT_DEVICE_OFLAG_RDWR);
            rt_device_control(tc_lcd_device, RTGRAPHIC_CTRL_GET_INFO, &info);

            lcd_width = info.width ;
            lcd_height = info.height ;
            lcd_align = info.draw_align;
        }
    }
#endif /* OUTPUT_TO_LCD */

    /* reset input,output,imageInfo */
    memset(&jpegIn, 0, sizeof(jpegIn));
    memset(&jpegOut, 0, sizeof(jpegOut));
    memset(&imageInfo, 0, sizeof(imageInfo));

    /* set default */
    bufferSize = 0;
    jpegIn.sliceMbSet = 0;
    jpegIn.bufferSize = 0;

#ifdef PP_PIPELINE_ENABLED

    /* Print API and build version numbers */
    decVer = JpegDecGetAPIVersion();
    decBuild = JpegDecGetBuild();

    /* Version */
    TC_JPEG_NANOD_LOG_I(
        "\nX170 JPEG Decoder API v%d.%d - SW build: %d - HW build: %x\n",
        decVer.major, decVer.minor, decBuild.swBuild, decBuild.hwBuild);

    /* Print API and build version numbers */
    ppVer = PPGetAPIVersion();
    ppBuild = PPGetBuild();

    /* Version */
    TC_JPEG_NANOD_LOG_I(
        "\nX170 PP API v%d.%d - SW build: %d - HW build: %x\n",
        ppVer.major, ppVer.minor, ppBuild.swBuild, ppBuild.hwBuild);

#endif

    /* check if 8170 HW */
    is8170HW = (decBuild.hwBuild >> 16) == 0x8170U ? 1 : 0;

    /* after thumnails done ==> decode full images */
startFullDecode:

    /******** PHASE 1 ********/
    TC_JPEG_NANOD_LOG_I("\nPHASE 1: INIT JPEG DECODER\n");

    /* Jpeg initialization */
    START_SW_PERFORMANCE;

    jpegRet = JpegDecInit(&jpeg);
    END_SW_PERFORMANCE;

    if (jpegRet != JPEGDEC_OK)
    {
        /* Handle here the error situation */
        PrintJpegRet(&jpegRet);
        goto end;
    }

#if 0//def PP_PIPELINE_ENABLED
    /* Initialize the post processer. If unsuccessful -> exit */
    if (pp_startup(ppInst, jpeg, PP_PIPELINED_DEC_TYPE_JPEG) != 0)
    {
        TC_JPEG_NANOD_LOG_I("PP INITIALIZATION FAILED\n");
        goto end;
    }
#endif
    START_SW_PERFORMANCE;

    JpegDecInitReg(jpeg);

    END_SW_PERFORMANCE;

    TC_JPEG_NANOD_LOG_I("PHASE 1: INIT JPEG DECODER successful\n");

    jpegIn.bufferSize = 0;

    /* initialize JpegDecDecode input structure */
    jpegIn.streamBuffer.pVirtualAddress = (u32 *) byteStrmStart;
    jpegIn.streamBuffer.busAddress = (g1_addr_t) byteStrmStart;
    jpegIn.streamLength = streamTotalLen;

    if (writeOutput)
        TC_JPEG_NANOD_LOG_I("\t-File: Write output: YES: %d\n", writeOutput);
    else
        TC_JPEG_NANOD_LOG_I("\t-File: Write output: NO: %d\n", writeOutput);
    TC_JPEG_NANOD_LOG_I("\t-File: MbRows/slice: %d\n", jpegIn.sliceMbSet);
    TC_JPEG_NANOD_LOG_I("\t-File: Buffer size: %d\n", jpegIn.bufferSize);
    TC_JPEG_NANOD_LOG_I("\t-File: Stream size: %d\n", jpegIn.streamLength);

    /* jump here is frames still left */
decode:

    /******** PHASE 3 ********/
    TC_JPEG_NANOD_LOG_I("\nPHASE 3: GET IMAGE INFO\n");
    START_SW_PERFORMANCE;
    jpegRet = FindImageInfoEnd(byteStrmStart, streamTotalLen, &imageInfoLength);
    END_SW_PERFORMANCE;

    LOG_I("\timageInfoLength %d\n", imageInfoLength);
    /* If image info is not found, do not corrupt the header */
    if (jpegRet != 0)
    {
        goto end;
    }

    /* Get image information of the JFIF and decode JFIF header */
    START_SW_PERFORMANCE;

    jpegRet = JpegDecGetImageInfo(jpeg, &jpegIn, &imageInfo);
    END_SW_PERFORMANCE;

    if (jpegRet != JPEGDEC_OK)
    {
        /* Handle here the error situation */
        PrintJpegRet(&jpegRet);
        if (JPEGDEC_INCREASE_INPUT_BUFFER == jpegRet)
        {
            TC_JPEG_NANOD_LOG_I("JPEGDEC_INCREASE_INPUT_BUFFER\n");
            goto end;
        }
        else
        {
            /* LOG_I JpegDecGetImageInfo() info */
            TC_JPEG_NANOD_LOG_I("\n\t--------------------------------------\n");
            TC_JPEG_NANOD_LOG_I("\tNOTE! IMAGE INFO WAS CHANGED!!!\n");
            TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n\n");
            PrintGetImageInfo(&imageInfo);
            TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n");

            /* check if MJPEG stream and Thumb decoding ==> continue to FULL */
            if (mode)
            {
                if (
                    imageInfo.outputWidthThumb == prevOutputWidthTn &&
                    imageInfo.outputHeightThumb == prevOutputHeightTn &&
                    imageInfo.outputFormatThumb == prevOutputFormatTn)
                {
                    TC_JPEG_NANOD_LOG_I("\n\t--------------------------------------\n");
                    TC_JPEG_NANOD_LOG_I("\tNOTE! THUMB INFO NOT CHANGED ==> DECODE!!!\n");
                    TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n\n");
                }
                else
                {
                    ThumbDone = 1;
                    goto end;
                }
            }
            else
            {
                /* if MJPEG and only THUMB changed ==> continue */
                if (imageInfo.outputWidth == prevOutputWidth &&
                        imageInfo.outputHeight == prevOutputHeight &&
                        imageInfo.outputFormat == prevOutputFormat)
                {
                    TC_JPEG_NANOD_LOG_I("\n\t--------------------------------------\n");
                    TC_JPEG_NANOD_LOG_I("\tNOTE! FULL IMAGE INFO NOT CHANGED ==> DECODE!!!\n");
                    TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n\n");
                }
                else
                {
                    goto end;
                }
            }
        }
    }

    /* save for MJPEG check */
    /* full */
    prevOutputWidth = imageInfo.outputWidth;
    prevOutputHeight = imageInfo.outputHeight;
    prevOutputFormat = imageInfo.outputFormat;
    /* thumbnail */
    prevOutputWidthTn = imageInfo.outputWidthThumb;
    prevOutputHeightTn = imageInfo.outputHeightThumb;
    prevOutputFormatTn = imageInfo.outputFormatThumb;

    /* LOG_I JpegDecGetImageInfo() info */
    PrintGetImageInfo(&imageInfo);

    /*  ******************** THUMBNAIL **************************** */
    /* Select if Thumbnail or full resolution image will be decoded */
    if (imageInfo.thumbnailType == JPEGDEC_THUMBNAIL_JPEG)
    {
        /* if all thumbnails processed (MJPEG) */
        if (!ThumbDone)
            jpegIn.decImageType = JPEGDEC_THUMBNAIL;
        else
            jpegIn.decImageType = JPEGDEC_IMAGE;

    }
    else if (imageInfo.thumbnailType == JPEGDEC_NO_THUMBNAIL)
        jpegIn.decImageType = JPEGDEC_IMAGE;
    else if (imageInfo.thumbnailType == JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT)
        jpegIn.decImageType = JPEGDEC_IMAGE;

    /* check if forced to decode only full resolution images
        ==> discard thumbnail */
    if (onlyFullResolution)
    {
        /* decode only full resolution image */
        TC_JPEG_NANOD_LOG_I(
            "\n\tNOTE! FORCED BY USER TO DECODE ONLY FULL RESOLUTION IMAGE\n");
        jpegIn.decImageType = JPEGDEC_IMAGE;
    }

    TC_JPEG_NANOD_LOG_I("PHASE 3: GET IMAGE INFO successful\n");

    /* TB SPECIFIC == LOOP IF THUMBNAIL IN JFIF */
    /* Decode JFIF */
    if (jpegIn.decImageType == JPEGDEC_THUMBNAIL)
        mode = 1; /* TODO KIMA */
    else
        mode = 0;

#ifdef ASIC_TRACE_SUPPORT
    /* Handle incorrect slice size for HW testing */
    if (jpegIn.sliceMbSet > (imageInfo.outputHeight >> 4))
    {
        jpegIn.sliceMbSet = (imageInfo.outputHeight >> 4);
        LOG_I("FIXED Decoder Slice MB Set %d\n", jpegIn.sliceMbSet);
    }
#endif

    /* no slice mode supported in progressive || non-interleaved ==> force to full mode */
    if ((jpegIn.decImageType == JPEGDEC_THUMBNAIL &&
            imageInfo.codingModeThumb == JPEGDEC_PROGRESSIVE) ||
            (jpegIn.decImageType == JPEGDEC_IMAGE &&
             imageInfo.codingMode == JPEGDEC_PROGRESSIVE))
        jpegIn.sliceMbSet = 0;

    /******** PHASE 4 ********/
    /* Image mode to decode */
    if (mode)
        TC_JPEG_NANOD_LOG_I("\nPHASE 4: DECODE FRAME: THUMBNAIL\n");
    else
        TC_JPEG_NANOD_LOG_I("\nPHASE 4: DECODE FRAME: FULL RESOLUTION\n");

    /* if input (only full, not tn) > 4096 MCU      */
    /* ==> force to slice mode                                      */
    if (mode == 0)
    {
        /* calculate MCU's */
        if (imageInfo.outputFormat == JPEGDEC_YCbCr400 ||
                imageInfo.outputFormat == JPEGDEC_YCbCr444_SEMIPLANAR)
        {
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 64);
            mcuInRow = (imageInfo.outputWidth / 8);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr420_SEMIPLANAR)
        {
            /* 265 is the amount of luma samples in MB for 4:2:0 */
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 256);
            mcuInRow = (imageInfo.outputWidth / 16);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr422_SEMIPLANAR)
        {
            /* 128 is the amount of luma samples in MB for 4:2:2 */
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 128);
            mcuInRow = (imageInfo.outputWidth / 16);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr440)
        {
            /* 128 is the amount of luma samples in MB for 4:4:0 */
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 128);
            mcuInRow = (imageInfo.outputWidth / 8);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr411_SEMIPLANAR)
        {
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 256);
            mcuInRow = (imageInfo.outputWidth / 32);
        }

        /* set mcuSizeDivider for slice size count */
        if (imageInfo.outputFormat == JPEGDEC_YCbCr400 ||
                imageInfo.outputFormat == JPEGDEC_YCbCr440 ||
                imageInfo.outputFormat == JPEGDEC_YCbCr444_SEMIPLANAR)
            mcuSizeDivider = 2;
        else
            mcuSizeDivider = 1;

        if (0 == jpegIn.sliceMbSet)
        {
            JpegDecUpdateSliceMbSet(jpeg, &jpegIn, &imageInfo, amountOfMCUs,
                                    mcuInRow, mcuSizeDivider);

            if (0 != jpegIn.sliceMbSet)
            {
                LOG_I("Force to slice mode ==> Decoder Slice MB Set %d\n",
                      jpegIn.sliceMbSet);
            }
        }

    }

#ifdef PP_PIPELINE_ENABLED
    /* Initialize the post processer. If unsuccessful -> exit */
    START_SW_PERFORMANCE;
    u32 ret = pp_startup2(jpeg, PP_PIPELINED_DEC_TYPE_JPEG, &imageInfo, outpu_cf, out_buf, out_buf_len);
    END_SW_PERFORMANCE;
    if (ret != 0)
    {
        TC_JPEG_NANOD_LOG_I("PP INITIALIZATION FAILED\n");
        goto end;
    }
#endif

    START_SW_PERFORMANCE;
    /* decode */
    do
    {

        jpegRet = JpegDecDecode(jpeg, &jpegIn, &jpegOut);

        if (jpegRet == JPEGDEC_FRAME_READY)
        {
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_FRAME_READY\n");

#if 0
            /* check if progressive ==> planar output */
            if ((imageInfo.codingMode == JPEGDEC_PROGRESSIVE && mode == 0) ||
                    (imageInfo.codingModeThumb == JPEGDEC_PROGRESSIVE &&
                     mode == 1))
            {
                progressive = 1;
            }

            if ((imageInfo.codingMode == JPEGDEC_NONINTERLEAVED && mode == 0)
                    || (imageInfo.codingModeThumb == JPEGDEC_NONINTERLEAVED &&
                        mode == 1))
                nonInterleaved = 1;
            else
                nonInterleaved = 0;

            if (jpegIn.sliceMbSet && fullSliceCounter == -1)
                slicedOutputUsed = 1;

            /* info to handleSlicedOutput */
            frameReady = 1;
            if (!mode)
                nbrOfImagesToOut++;
#endif /* 0 */

            /* for input buffering */
            prevRet = JPEGDEC_FRAME_READY;
        }
        else if (jpegRet == JPEGDEC_SCAN_PROCESSED)
        {
            /* TODO! Progressive scan ready... */
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_SCAN_PROCESSED\n");
            goto end;
        }
        else if (jpegRet == JPEGDEC_SLICE_READY)
        {
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_SLICE_READY\n");
            goto end;
        }
        else if (jpegRet == JPEGDEC_STRM_PROCESSED)
        {
            TC_JPEG_NANOD_LOG_I(
                "\t-JPEG: JPEGDEC_STRM_PROCESSED ==> Load input buffer\n");
            goto end;
        }
        else if (jpegRet == JPEGDEC_STRM_ERROR)
        {
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_STRM_ERROR\n");

            goto end;
        }
        else
        {
            /* Handle here the error situation */
            PrintJpegRet(&jpegRet);
            goto end;
        }

    }
    while (jpegRet != JPEGDEC_FRAME_READY);

    END_SW_PERFORMANCE;

#if 0

error:

    /* calculate/write output of slice */
    if (slicedOutputUsed && jpegOut.outputPictureY.pVirtualAddress != NULL)
    {
        handleSlicedOutput(&imageInfo, &jpegIn, &jpegOut);
        slicedOutputUsed = 0;
    }

    if (jpegOut.outputPictureY.pVirtualAddress != NULL)
    {
        /* calculate size for output */
        calcSize(&imageInfo, mode);

    }
#endif /* 0 */

    /* Thumbnail || full resolution */
    if (!mode)
        TC_JPEG_NANOD_LOG_I("\n\t-JPEG: ++++++++++ FULL RESOLUTION ++++++++++\n");
    else
        TC_JPEG_NANOD_LOG_I("\t-JPEG: ++++++++++ THUMBNAIL ++++++++++\n");

    TC_JPEG_NANOD_LOG_I("\t-JPEG: Instance %x\n", (JpegDecContainer *) jpeg);
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Luma output: 0x%16x size: 0x%x\n",
                        jpegOut.outputPictureY.pVirtualAddress, GetLumaSize(&imageInfo, mode));
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Chroma output: 0x%16x size: 0x%x\n",
                        jpegOut.outputPictureCbCr.pVirtualAddress, GetChromaSize(&imageInfo, mode));
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Luma output bus: 0x%16x\n",
                        (u8 *) jpegOut.outputPictureY.busAddress);
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Chroma output bus: 0x%16x\n",
                        (u8 *) jpegOut.outputPictureCbCr.busAddress);

    TC_JPEG_NANOD_LOG_I("PHASE 4: DECODE FRAME successful\n");

    /******** PHASE 5 ********/
    TC_JPEG_NANOD_LOG_I("\nPHASE 5: OUTPUT\n");

#ifdef PP_PIPELINE_ENABLED
    /* PP test bench will do the operations only if enabled */
    /*pp_set_rotation(); */

    TC_JPEG_NANOD_LOG_I("\t-JPEG: PP_OUTPUT_WRITE\n");
    START_SW_PERFORMANCE;
    pp_api_wait();
    END_SW_PERFORMANCE;
#endif

#if 0//def OUTPUT_TO_LCD
    {

        uint16_t x, y;

        uint16_t cf;

        switch (ppConf.ppOutImg.pixFormat)
        {
        case PP_PIX_FMT_RGB16_CUSTOM:
        case PP_PIX_FMT_RGB16_5_6_5:
        case PP_PIX_FMT_BGR16_5_6_5:
            cf = RTGRAPHIC_PIXEL_FORMAT_RGB565;
            break;
        case PP_PIX_FMT_RGB32_CUSTOM:
        case PP_PIX_FMT_RGB32:
        case PP_PIX_FMT_BGR32:
            cf = RTGRAPHIC_PIXEL_FORMAT_ARGB888;
            break;
        default:
            RT_ASSERT(0);
            break;
        }

        rt_device_control(tc_lcd_device, RTGRAPHIC_CTRL_SET_BUF_FORMAT, &cf);

        if ((lcd_width  >= ppConf.ppOutImg.width) && (lcd_height  >= ppConf.ppOutImg.height))
        {
            x = RT_ALIGN_DOWN((lcd_width  - ppConf.ppOutImg.width)  >> 1, lcd_align);
            y = RT_ALIGN_DOWN((lcd_height - ppConf.ppOutImg.height) >> 1, lcd_align);
        }
        else
        {
            x = 0;
            y = 0;
        }

        TC_JPEG_PP_LOG_I("Output to LCD %x, [%d,%d,%d,%d]",
                         ppConf.ppOutImg.bufferBusAddr,
                         x,
                         y,
                         x + ppConf.ppOutImg.width - 1,
                         y + ppConf.ppOutImg.height - 1);

        rt_graphix_ops(tc_lcd_device)->set_window(
            x,
            y,
            (lcd_width  >= ppConf.ppOutImg.width) ? (x + ppConf.ppOutImg.width - 1) : (x + lcd_width - 1),
            (lcd_height  >= ppConf.ppOutImg.height) ? (y + ppConf.ppOutImg.height - 1) : (y + lcd_height - 1)
        );

        rt_graphix_ops(tc_lcd_device)->draw_rect(
            (const char *)ppConf.ppOutImg.bufferBusAddr,
            x,
            y,
            x + ppConf.ppOutImg.width - 1,
            y + ppConf.ppOutImg.height - 1);

        //rt_thread_mdelay(3000);
    }
#endif /* OUTPUT_TO_LCD */

    return_status = streamTotalLen;

end:

    /******** PHASE 6 ********/
    TC_JPEG_NANOD_LOG_I("\nPHASE 6: RELEASE JPEG DECODER\n");

    /* release decoder instance */
    START_SW_PERFORMANCE;
#ifdef PP_PIPELINE_ENABLED
    pp_api_release2(jpeg);
#endif

#if 0
    if (streamMem.virtualAddress != NULL)
        DWLFreeLinear(((JpegDecContainer *) jpeg)->dwl, &streamMem);

    if (userAllocLuma.virtualAddress != NULL)
        DWLFreeRefFrm(((JpegDecContainer *) jpeg)->dwl, &userAllocLuma);

    if (userAllocChroma.virtualAddress != NULL)
        DWLFreeRefFrm(((JpegDecContainer *) jpeg)->dwl, &userAllocChroma);

    if (userAllocCr.virtualAddress != NULL)
        DWLFreeRefFrm(((JpegDecContainer *) jpeg)->dwl, &userAllocCr);
#endif /* 0 */

    JpegDecRelease(jpeg);
    END_SW_PERFORMANCE;

    TC_JPEG_NANOD_LOG_I("PHASE 6: RELEASE JPEG DECODER successful\n\n");
#if 0

    /* check if (thumbnail + full) ==> decode all full images */
    if (ThumbDone && nbrOfImages)
    {
        prevRet = JPEGDEC_STRM_ERROR;
        goto startFullDecode;
    }
#endif /* 0 */

    mpu_dcache_invalidate((void *) out_buf, (int32_t) out_buf_len);

    //DWLFreeLinear(((JpegDecContainer *) jpeg)->dwl, &streamMem);

    FINALIZE_SW_PERFORMANCE;

    TC_JPEG_NANOD_LOG_I("TB: ...released\n");

return_:

    return return_status;

}

int jpeg_nanod_decode_init(void)
{
    return 0;

}

int jpeg_nanod_decode_get_dimension(uint8_t *byteStrmStart, uint32_t streamTotalLen, uint32_t *width, uint32_t *height)
{
    i32 i, j = 0;
    u32 tmp = 0;
    u32 size = 0;
    u32 loop;
    u32 frameCounter = 0;
    u32 inputReadType = 0;
    i32 bufferSize = 0;

    JpegDecApiVersion decVer;
    JpegDecBuild decBuild;

#ifdef PP_PIPELINE_ENABLED
    PPApiVersion ppVer;
    PPBuild ppBuild;
#endif

    u32 streamHeaderCorrupt = 0;
    u32 seedRnd = 0;
    u32 streamBitSwap = 0;
    u32 streamTruncate = 0;

    JpegDecRet jpegRet;

    u32 imageInfoLength = 0;
    u32 writeOutput = 0;
    static u32 mode = 0; //0 - thumbnail  1-full solution
    u32 ThumbDone = 0;
    u32 onlyFullResolution = 0;
    u32 mcuSizeDivider = 0;
    u32 amountOfMCUs = 0;
    u32 mcuInRow = 0;
    int r;

    INIT_SW_PERFORMANCE;
    
    /* reset input,output,imageInfo */
    memset(&jpegIn, 0, sizeof(jpegIn));
    memset(&jpegOut, 0, sizeof(jpegOut));
    memset(&imageInfo, 0, sizeof(imageInfo));

    /* set default */
    bufferSize = 0;
    jpegIn.sliceMbSet = 0;
    jpegIn.bufferSize = 0;

#ifdef PP_PIPELINE_ENABLED
    /* Print API and build version numbers */
    decVer = JpegDecGetAPIVersion();
    decBuild = JpegDecGetBuild();

    /* Version */
    TC_JPEG_NANOD_LOG_I(
        "\nX170 JPEG Decoder API v%d.%d - SW build: %d - HW build: %x\n",
        decVer.major, decVer.minor, decBuild.swBuild, decBuild.hwBuild);

    /* Print API and build version numbers */
    ppVer = PPGetAPIVersion();
    ppBuild = PPGetBuild();

    /* Version */
    TC_JPEG_NANOD_LOG_I(
        "\nX170 PP API v%d.%d - SW build: %d - HW build: %x\n",
        ppVer.major, ppVer.minor, ppBuild.swBuild, ppBuild.hwBuild);

#endif

    /* check if 8170 HW */
    is8170HW = (decBuild.hwBuild >> 16) == 0x8170U ? 1 : 0;

    /* after thumnails done ==> decode full images */
startFullDecode:

    /******** PHASE 1 ********/
    TC_JPEG_NANOD_LOG_I("\nPHASE 1: INIT JPEG DECODER\n");

    /* Jpeg initialization */
    START_SW_PERFORMANCE;

    jpegRet = JpegDecInit(&jpeg);
    END_SW_PERFORMANCE;

    if (jpegRet != JPEGDEC_OK)
    {
        /* Handle here the error situation */
        PrintJpegRet(&jpegRet);
        goto __init_end;
    }

#if 0//def PP_PIPELINE_ENABLED
    /* Initialize the post processer. If unsuccessful -> exit */
    if (pp_startup(ppInst, jpeg, PP_PIPELINED_DEC_TYPE_JPEG) != 0)
    {
        TC_JPEG_NANOD_LOG_I("PP INITIALIZATION FAILED\n");
        goto __init_end;
    }
#endif

    JpegDecInitReg(jpeg);

    TC_JPEG_NANOD_LOG_I("PHASE 1: INIT JPEG DECODER successful\n");
                
__init_end:                

    jpegIn.bufferSize = 0;

    /* initialize JpegDecDecode input structure */
    jpegIn.streamBuffer.pVirtualAddress = (u32 *) byteStrmStart;
    jpegIn.streamBuffer.busAddress = (g1_addr_t) byteStrmStart;
    jpegIn.streamLength = streamTotalLen;
    
    if (writeOutput)
        TC_JPEG_NANOD_LOG_I("\t-File: Write output: YES: %d\n", writeOutput);
    else
        TC_JPEG_NANOD_LOG_I("\t-File: Write output: NO: %d\n", writeOutput);
    TC_JPEG_NANOD_LOG_I("\t-File: MbRows/slice: %d\n", jpegIn.sliceMbSet);
    TC_JPEG_NANOD_LOG_I("\t-File: Buffer size: %d\n", jpegIn.bufferSize);
    TC_JPEG_NANOD_LOG_I("\t-File: Stream size: %d\n", jpegIn.streamLength);

    /* jump here is frames still left */
__decode:

    /******** PHASE 3 ********/
    TC_JPEG_NANOD_LOG_I("\nPHASE 3: GET IMAGE INFO\n");

    jpegRet = FindImageInfoEnd(byteStrmStart, streamTotalLen, &imageInfoLength);
    LOG_I("\timageInfoLength %d\n", imageInfoLength);
    /* If image info is not found, do not corrupt the header */
    if (jpegRet != 0)
    {

    }

    /* Get image information of the JFIF and decode JFIF header */
    START_SW_PERFORMANCE;

    jpegRet = JpegDecGetImageInfo(jpeg, &jpegIn, &imageInfo);
    END_SW_PERFORMANCE;

    if (jpegRet != JPEGDEC_OK)
    {
        /* Handle here the error situation */
        PrintJpegRet(&jpegRet);
        if (JPEGDEC_INCREASE_INPUT_BUFFER == jpegRet)
        {
            TC_JPEG_NANOD_LOG_I("JPEGDEC_INCREASE_INPUT_BUFFER\n");
            goto __get_dim_end;
        }
        else
        {
            /* LOG_I JpegDecGetImageInfo() info */
            TC_JPEG_NANOD_LOG_I("\n\t--------------------------------------\n");
            TC_JPEG_NANOD_LOG_I("\tNOTE! IMAGE INFO WAS CHANGED!!!\n");
            TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n\n");
            PrintGetImageInfo(&imageInfo);
            TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n");

            /* check if MJPEG stream and Thumb decoding ==> continue to FULL */
            if (mode)
            {
                if (
                    imageInfo.outputWidthThumb == prevOutputWidthTn &&
                    imageInfo.outputHeightThumb == prevOutputHeightTn &&
                    imageInfo.outputFormatThumb == prevOutputFormatTn)
                {
                    TC_JPEG_NANOD_LOG_I("\n\t--------------------------------------\n");
                    TC_JPEG_NANOD_LOG_I("\tNOTE! THUMB INFO NOT CHANGED ==> DECODE!!!\n");
                    TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n\n");
                }
                else
                {
                    ThumbDone = 1;
                    goto __get_dim_end;
                }
            }
            else
            {
                /* if MJPEG and only THUMB changed ==> continue */
                if (imageInfo.outputWidth == prevOutputWidth &&
                        imageInfo.outputHeight == prevOutputHeight &&
                        imageInfo.outputFormat == prevOutputFormat)
                {
                    TC_JPEG_NANOD_LOG_I("\n\t--------------------------------------\n");
                    TC_JPEG_NANOD_LOG_I("\tNOTE! FULL IMAGE INFO NOT CHANGED ==> DECODE!!!\n");
                    TC_JPEG_NANOD_LOG_I("\t--------------------------------------\n\n");
                }
                else
                {
                    goto __get_dim_end;
                }
            }
        }
    }

    /* save for MJPEG check */
    /* full */
    prevOutputWidth = imageInfo.outputWidth;
    prevOutputHeight = imageInfo.outputHeight;
    prevOutputFormat = imageInfo.outputFormat;
    /* thumbnail */
    prevOutputWidthTn = imageInfo.outputWidthThumb;
    prevOutputHeightTn = imageInfo.outputHeightThumb;
    prevOutputFormatTn = imageInfo.outputFormatThumb;

    /* LOG_I JpegDecGetImageInfo() info */
    PrintGetImageInfo(&imageInfo);

    /*  ******************** THUMBNAIL **************************** */
    /* Select if Thumbnail or full resolution image will be decoded */
    if (imageInfo.thumbnailType == JPEGDEC_THUMBNAIL_JPEG)
    {
        /* if all thumbnails processed (MJPEG) */
        if (!ThumbDone)
            jpegIn.decImageType = JPEGDEC_THUMBNAIL;
        else
            jpegIn.decImageType = JPEGDEC_IMAGE;

    }
    else if (imageInfo.thumbnailType == JPEGDEC_NO_THUMBNAIL)
        jpegIn.decImageType = JPEGDEC_IMAGE;
    else if (imageInfo.thumbnailType == JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT)
        jpegIn.decImageType = JPEGDEC_IMAGE;

    /* check if forced to decode only full resolution images
        ==> discard thumbnail */
    if (onlyFullResolution)
    {
        /* decode only full resolution image */
        TC_JPEG_NANOD_LOG_I(
            "\n\tNOTE! FORCED BY USER TO DECODE ONLY FULL RESOLUTION IMAGE\n");
        jpegIn.decImageType = JPEGDEC_IMAGE;
    }

    TC_JPEG_NANOD_LOG_I("PHASE 3: GET IMAGE INFO successful\n");

    /* TB SPECIFIC == LOOP IF THUMBNAIL IN JFIF */
    /* Decode JFIF */
    if (jpegIn.decImageType == JPEGDEC_THUMBNAIL)
        mode = 1; /* TODO KIMA */
    else
        mode = 0;

#ifdef ASIC_TRACE_SUPPORT
    /* Handle incorrect slice size for HW testing */
    if (jpegIn.sliceMbSet > (imageInfo.outputHeight >> 4))
    {
        jpegIn.sliceMbSet = (imageInfo.outputHeight >> 4);
        LOG_I("FIXED Decoder Slice MB Set %d\n", jpegIn.sliceMbSet);
    }
#endif

    /* no slice mode supported in progressive || non-interleaved ==> force to full mode */
    if ((jpegIn.decImageType == JPEGDEC_THUMBNAIL &&
            imageInfo.codingModeThumb == JPEGDEC_PROGRESSIVE) ||
            (jpegIn.decImageType == JPEGDEC_IMAGE &&
             imageInfo.codingMode == JPEGDEC_PROGRESSIVE))
        jpegIn.sliceMbSet = 0;

    /******** PHASE 4 ********/
    /* Image mode to decode */
    if (mode)
        TC_JPEG_NANOD_LOG_I("\nPHASE 4: DECODE FRAME: THUMBNAIL\n");
    else
        TC_JPEG_NANOD_LOG_I("\nPHASE 4: DECODE FRAME: FULL RESOLUTION\n");

    /* if input (only full, not tn) > 4096 MCU      */
    /* ==> force to slice mode                                      */
    if (mode == 0)
    {
        /* calculate MCU's */
        if (imageInfo.outputFormat == JPEGDEC_YCbCr400 ||
                imageInfo.outputFormat == JPEGDEC_YCbCr444_SEMIPLANAR)
        {
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 64);
            mcuInRow = (imageInfo.outputWidth / 8);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr420_SEMIPLANAR)
        {
            /* 265 is the amount of luma samples in MB for 4:2:0 */
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 256);
            mcuInRow = (imageInfo.outputWidth / 16);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr422_SEMIPLANAR)
        {
            /* 128 is the amount of luma samples in MB for 4:2:2 */
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 128);
            mcuInRow = (imageInfo.outputWidth / 16);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr440)
        {
            /* 128 is the amount of luma samples in MB for 4:4:0 */
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 128);
            mcuInRow = (imageInfo.outputWidth / 8);
        }
        else if (imageInfo.outputFormat == JPEGDEC_YCbCr411_SEMIPLANAR)
        {
            amountOfMCUs =
                ((imageInfo.outputWidth * imageInfo.outputHeight) / 256);
            mcuInRow = (imageInfo.outputWidth / 32);
        }

        /* set mcuSizeDivider for slice size count */
        if (imageInfo.outputFormat == JPEGDEC_YCbCr400 ||
                imageInfo.outputFormat == JPEGDEC_YCbCr440 ||
                imageInfo.outputFormat == JPEGDEC_YCbCr444_SEMIPLANAR)
            mcuSizeDivider = 2;
        else
            mcuSizeDivider = 1;

        if (0 == jpegIn.sliceMbSet)
        {
            JpegDecUpdateSliceMbSet(jpeg, &jpegIn, &imageInfo, amountOfMCUs,
                                    mcuInRow, mcuSizeDivider);

            if (0 != jpegIn.sliceMbSet)
            {
                LOG_I("Force to slice mode ==> Decoder Slice MB Set %d\n",
                      jpegIn.sliceMbSet);
            }
        }
    }

    *width = imageInfo.outputWidth;
    *height = imageInfo.outputHeight;
    
    r = 0;
    goto __deinit_start;

__get_dim_end:
    *width = 0;
    *height = 0;
    r= 1;

__deinit_start:
    TC_JPEG_NANOD_LOG_I("\nPHASE 6: RELEASE JPEG DECODER\n");

    /* release decoder instance */
    START_SW_PERFORMANCE;
    JpegDecRelease(jpeg);
    END_SW_PERFORMANCE;

    TC_JPEG_NANOD_LOG_I("PHASE 6: RELEASE JPEG DECODER successful\n\n");

    FINALIZE_SW_PERFORMANCE;

    return r;

}

int jpeg_nanod_decode(uint8_t *byteStrmStart, 
            uint32_t streamTotalLen, 
            const char *out_cf,
            uint8_t *out_buf,
            uint32_t out_buf_len)
{
    int return_status = 0;

    JpegDecRet jpegRet;
    u32 outpu_cf = str2Jpegcf(out_cf);

    static u32 mode = 0; //0 - thumbnail  1-full solution

    INIT_SW_PERFORMANCE;

#ifdef PP_PIPELINE_ENABLED
    /* Initialize the post processer. If unsuccessful -> exit */
    if (pp_startup2(jpeg, PP_PIPELINED_DEC_TYPE_JPEG, &imageInfo, outpu_cf, out_buf, out_buf_len) != 0)
    {
        TC_JPEG_NANOD_LOG_I("PP INITIALIZATION FAILED\n");
        goto end;
    }
#endif

    /* decode */
    do
    {
        START_SW_PERFORMANCE;

        jpegRet = JpegDecDecode(jpeg, &jpegIn, &jpegOut);
        END_SW_PERFORMANCE;

        if (jpegRet == JPEGDEC_FRAME_READY)
        {
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_FRAME_READY\n");

#if 0
            /* check if progressive ==> planar output */
            if ((imageInfo.codingMode == JPEGDEC_PROGRESSIVE && mode == 0) ||
                    (imageInfo.codingModeThumb == JPEGDEC_PROGRESSIVE &&
                     mode == 1))
            {
                progressive = 1;
            }

            if ((imageInfo.codingMode == JPEGDEC_NONINTERLEAVED && mode == 0)
                    || (imageInfo.codingModeThumb == JPEGDEC_NONINTERLEAVED &&
                        mode == 1))
                nonInterleaved = 1;
            else
                nonInterleaved = 0;

            if (jpegIn.sliceMbSet && fullSliceCounter == -1)
                slicedOutputUsed = 1;

            /* info to handleSlicedOutput */
            frameReady = 1;
            if (!mode)
                nbrOfImagesToOut++;
#endif /* 0 */

            /* for input buffering */
            //prevRet = JPEGDEC_FRAME_READY;
        }
        else if (jpegRet == JPEGDEC_SCAN_PROCESSED)
        {
            /* TODO! Progressive scan ready... */
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_SCAN_PROCESSED\n");
            RT_ASSERT(0);
        }
        else if (jpegRet == JPEGDEC_SLICE_READY)
        {
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_SLICE_READY\n");
            RT_ASSERT(0);
        }
        else if (jpegRet == JPEGDEC_STRM_PROCESSED)
        {
            TC_JPEG_NANOD_LOG_I(
                "\t-JPEG: JPEGDEC_STRM_PROCESSED ==> Load input buffer\n");
            RT_ASSERT(0);
        }
        else if (jpegRet == JPEGDEC_STRM_ERROR)
        {
            TC_JPEG_NANOD_LOG_I("\t-JPEG: JPEGDEC_STRM_ERROR\n");

            RT_ASSERT(0);

        }
        else
        {
            /* Handle here the error situation */
            PrintJpegRet(&jpegRet);
            goto end;
        }

    }
    while (jpegRet != JPEGDEC_FRAME_READY);

#if 0

error:

    /* calculate/write output of slice */
    if (slicedOutputUsed && jpegOut.outputPictureY.pVirtualAddress != NULL)
    {
        handleSlicedOutput(&imageInfo, &jpegIn, &jpegOut);
        slicedOutputUsed = 0;
    }

    if (jpegOut.outputPictureY.pVirtualAddress != NULL)
    {
        /* calculate size for output */
        calcSize(&imageInfo, mode);

    }
#endif /* 0 */

    /* Thumbnail || full resolution */
    if (!mode)
        TC_JPEG_NANOD_LOG_I("\n\t-JPEG: ++++++++++ FULL RESOLUTION ++++++++++\n");
    else
        TC_JPEG_NANOD_LOG_I("\t-JPEG: ++++++++++ THUMBNAIL ++++++++++\n");

    TC_JPEG_NANOD_LOG_I("\t-JPEG: Instance %x\n", (JpegDecContainer *) jpeg);
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Luma output: 0x%16x size: 0x%x\n",
                        jpegOut.outputPictureY.pVirtualAddress, GetLumaSize(&imageInfo, mode));
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Chroma output: 0x%16x size: 0x%x\n",
                        jpegOut.outputPictureCbCr.pVirtualAddress, GetChromaSize(&imageInfo, mode));
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Luma output bus: 0x%16x\n",
                        (u8 *) jpegOut.outputPictureY.busAddress);
    TC_JPEG_NANOD_LOG_I("\t-JPEG: Chroma output bus: 0x%16x\n",
                        (u8 *) jpegOut.outputPictureCbCr.busAddress);

    TC_JPEG_NANOD_LOG_I("PHASE 4: DECODE FRAME successful\n");

    /******** PHASE 5 ********/
    TC_JPEG_NANOD_LOG_I("\nPHASE 5: OUTPUT\n");

#ifdef PP_PIPELINE_ENABLED
    /* PP test bench will do the operations only if enabled */
    /*pp_set_rotation(); */

    TC_JPEG_NANOD_LOG_I("\t-JPEG: PP_OUTPUT_WRITE\n");
    pp_api_wait();

#endif

#if 0//def OUTPUT_TO_LCD
    {

        uint16_t x, y;

        uint16_t cf;

        switch (ppConf.ppOutImg.pixFormat)
        {
        case PP_PIX_FMT_RGB16_CUSTOM:
        case PP_PIX_FMT_RGB16_5_6_5:
        case PP_PIX_FMT_BGR16_5_6_5:
            cf = RTGRAPHIC_PIXEL_FORMAT_RGB565;
            break;
        case PP_PIX_FMT_RGB32_CUSTOM:
        case PP_PIX_FMT_RGB32:
        case PP_PIX_FMT_BGR32:
            cf = RTGRAPHIC_PIXEL_FORMAT_ARGB888;
            break;
        default:
            RT_ASSERT(0);
            break;
        }

        rt_device_control(tc_lcd_device, RTGRAPHIC_CTRL_SET_BUF_FORMAT, &cf);

        if ((lcd_width  >= ppConf.ppOutImg.width) && (lcd_height  >= ppConf.ppOutImg.height))
        {
            x = RT_ALIGN_DOWN((lcd_width  - ppConf.ppOutImg.width)  >> 1, lcd_align);
            y = RT_ALIGN_DOWN((lcd_height - ppConf.ppOutImg.height) >> 1, lcd_align);
        }
        else
        {
            x = 0;
            y = 0;
        }

        TC_JPEG_PP_LOG_I("Output to LCD %x, [%d,%d,%d,%d]",
                         ppConf.ppOutImg.bufferBusAddr,
                         x,
                         y,
                         x + ppConf.ppOutImg.width - 1,
                         y + ppConf.ppOutImg.height - 1);

        rt_graphix_ops(tc_lcd_device)->set_window(
            x,
            y,
            (lcd_width  >= ppConf.ppOutImg.width) ? (x + ppConf.ppOutImg.width - 1) : (x + lcd_width - 1),
            (lcd_height  >= ppConf.ppOutImg.height) ? (y + ppConf.ppOutImg.height - 1) : (y + lcd_height - 1)
        );

        rt_graphix_ops(tc_lcd_device)->draw_rect(
            (const char *)ppConf.ppOutImg.bufferBusAddr,
            x,
            y,
            x + ppConf.ppOutImg.width - 1,
            y + ppConf.ppOutImg.height - 1);

        //rt_thread_mdelay(3000);
    }
#endif /* OUTPUT_TO_LCD */

#ifdef PP_PIPELINE_ENABLED
    pp_api_release2(jpeg);
#endif

TC_JPEG_NANOD_LOG_I("\t-JPEG: DECODE DONE\n");

end:

    return_status = 0;

return_:

    UNUSED(return_status);
    return (int)streamTotalLen;

}

int jpeg_nanod_decode_deinit(void)
{
    return 0;
}

#endif

