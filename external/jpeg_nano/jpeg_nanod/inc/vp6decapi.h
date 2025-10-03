/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2011 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
-
-  Description : VP6 Decoder API
-
------------------------------------------------------------------------------*/

#ifndef __VP6DECAPI_H__
#define __VP6DECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"
#include "decapicommon.h"
#ifdef USE_EXTERNAL_BUFFER
#include "dwl.h"
#endif

/*------------------------------------------------------------------------------
    API type definitions
------------------------------------------------------------------------------*/
/* Return values */
typedef enum VP6DecRet_
{
    VP6DEC_OK = 0,
    VP6DEC_STRM_PROCESSED = 1,
    VP6DEC_PIC_RDY = 2,
    VP6DEC_PIC_DECODED = 3,
    VP6DEC_HDRS_RDY = 4,
    VP6DEC_ADVANCED_TOOLS = 5,
#ifdef USE_OUTPUT_RELEASE
    VP6DEC_ABORTED = 7,
    VP6DEC_END_OF_STREAM = 8,
#endif

#ifdef USE_EXTERNAL_BUFFER
    /** Waiting for external buffers allocated. */
    VP6DEC_WAITING_FOR_BUFFER = 9,
#endif
#ifdef USE_OUTPUT_RELEASE
    VP6DEC_FLUSHED = 10,
#endif

    VP6DEC_PARAM_ERROR = -1,
    VP6DEC_STRM_ERROR = -2,
    VP6DEC_NOT_INITIALIZED = -3,
    VP6DEC_MEMFAIL = -4,
    VP6DEC_INITFAIL = -5,
    VP6DEC_HDRS_NOT_RDY = -6,
    VP6DEC_STREAM_NOT_SUPPORTED = -8,
#ifdef USE_EXTERNAL_BUFFER
    VP6DEC_EXT_BUFFER_REJECTED = -9, /**<\hideinitializer */
#endif

    VP6DEC_HW_RESERVED = -254,
    VP6DEC_HW_TIMEOUT = -255,
    VP6DEC_HW_BUS_ERROR = -256,
    VP6DEC_SYSTEM_ERROR = -257,
    VP6DEC_DWL_ERROR = -258,

    VP6DEC_EVALUATION_LIMIT_EXCEEDED = -999,
    VP6DEC_FORMAT_NOT_SUPPORTED = -1000
} VP6DecRet;

/* decoder  output Frame format */
typedef enum VP6DecOutFormat_
{
    VP6DEC_SEMIPLANAR_YUV420 = 0x020001,
    VP6DEC_TILED_YUV420 = 0x020002
} VP6DecOutFormat;

/* Input structure */
typedef struct VP6DecInput_
{
    const u8 *pStream;   /* Pointer to the input data */
    g1_addr_t streamBusAddress;   /* DMA bus address of the input stream */
    u32 dataLen;         /* Number of bytes to be decoded         */
    u32 picId;
} VP6DecInput;

/* Output structure */
typedef struct VP6DecOutput_
{
    u32 dataLeft;
} VP6DecOutput;

#define VP6_SCALE_MAINTAIN_ASPECT_RATIO     0
#define VP6_SCALE_TO_FIT                    1
#define VP6_SCALE_CENTER                    2
#define VP6_SCALE_OTHER                     3

/* stream info filled by VP6DecGetInfo */
typedef struct VP6DecInfo_
{
    u32 vp6Version;
    u32 vp6Profile;
#ifdef USE_EXTERNAL_BUFFER
    u32 picBuffSize;
#endif
    u32 frameWidth;      /* coded width */
    u32 frameHeight;     /* coded height */
    u32 scaledWidth;     /* scaled width of the displayed video */
    u32 scaledHeight;    /* scaled height of the displayed video */
    u32 scalingMode;     /* way to scale the frame to output */
    DecDpbMode dpbMode;             /* DPB mode; frame, or field interlaced */
    VP6DecOutFormat outputFormat;   /* format of the output frame */
} VP6DecInfo;

/* Version information */
typedef struct VP6DecApiVersion_
{
    u32 major;           /* API major version */
    u32 minor;           /* API minor version */
} VP6DecApiVersion;

typedef struct DecSwHwBuild_  VP6DecBuild;

/* Output structure for VP6DecNextPicture */
typedef struct VP6DecPicture_
{
    u32 frameWidth;        /* pixels width of the frame as stored in memory */
    u32 frameHeight;       /* pixel height of the frame as stored in memory */
    const u32 *pOutputFrame;    /* Pointer to the frame */
    g1_addr_t outputFrameBusAddress;  /* DMA bus address of the output frame buffer */
    u32 picId;           /* Identifier of the Frame to be displayed */
    u32 decodeId;
    u32 picCodingType;   /* Picture coding type */
    u32 isIntraFrame;    /* Indicates if Frame is an Intra Frame */
    u32 isGoldenFrame;   /* Indicates if Frame is a Golden reference Frame */
    u32 nbrOfErrMBs;     /* Number of concealed MB's in the frame  */
    DecOutFrmFormat outputFormat;
} VP6DecPicture;
#ifdef USE_EXTERNAL_BUFFER
/*!\struct VP6DecBufferInfo_
 * \brief Reference buffer information
 *
 * A structure containing the reference buffer information, filled by
 * VP6DecGetBufferInfo()
 *
 * \typedef VP6DecBufferInfo
 * A typename for #VP6DecBufferInfo_.
 */
typedef struct VP6DecBufferInfo_
{
    u32 nextBufSize;
    u32 bufNum;
    DWLLinearMem_t bufToFree;
#ifdef ASIC_TRACE_SUPPORT
    u32 is_frame_buffer;
#endif
} VP6DecBufferInfo;
#endif

/* Decoder instance */
typedef const void *VP6DecInst;

/*------------------------------------------------------------------------------
    Prototypes of Decoder API functions
------------------------------------------------------------------------------*/
VP6DecApiVersion VP6DecGetAPIVersion(void);

VP6DecBuild VP6DecGetBuild(void);

VP6DecRet VP6DecInit(VP6DecInst *pDecInst,
#ifdef USE_EXTERNAL_BUFFER
                     const void *dwl,
#endif
                     DecErrorHandling errorHandling,
                     u32 numFrameBuffers,
                     DecDpbFlags dpbFlags,
                     u32 useAdaptiveBuffers,
                     u32 nGuardSize);

void VP6DecRelease(VP6DecInst decInst);

VP6DecRet VP6DecDecode(VP6DecInst decInst,
                       const VP6DecInput *pInput, VP6DecOutput *pOutput);

VP6DecRet VP6DecNextPicture(VP6DecInst decInst,
                            VP6DecPicture *pOutput, u32 endOfStream);
#ifdef USE_OUTPUT_RELEASE
VP6DecRet VP6DecPictureConsumed(VP6DecInst decInst, VP6DecPicture *pOutput);

VP6DecRet VP6DecEndOfStream(VP6DecInst decInst, u32 strmEndFlag);

VP6DecRet VP6DecAbort(VP6DecInst decInst);

VP6DecRet VP6DecAbortAfter(VP6DecInst decInst);
#endif

VP6DecRet VP6DecGetInfo(VP6DecInst decInst, VP6DecInfo *pDecInfo);

VP6DecRet VP6DecPeek(VP6DecInst decInst, VP6DecPicture *pOutput);
#ifdef USE_EXTERNAL_BUFFER
VP6DecRet VP6DecGetBufferInfo(VP6DecInst decInst, VP6DecBufferInfo *mem_info);

VP6DecRet VP6DecAddBuffer(VP6DecInst decInst, DWLLinearMem_t *info);
#endif

/*------------------------------------------------------------------------------
    Prototype of the API trace funtion. Traces all API entries and returns.
    This must be implemented by the application using the decoder API!
    Argument:
        string - trace message, a null terminated string
------------------------------------------------------------------------------*/
void VP6DecTrace(const char *string);

#ifdef __cplusplus
}
#endif

#endif                       /* __VP6DECAPI_H__ */
