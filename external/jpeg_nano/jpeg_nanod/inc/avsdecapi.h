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
--
--  Description : API for the 8190 AVS Decoder
--
------------------------------------------------------------------------------*/

#ifndef __AVSDECAPI_H__
#define __AVSDECAPI_H__

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
typedef enum
{
    AVSDEC_OK = 0,
    AVSDEC_STRM_PROCESSED = 1,
    AVSDEC_PIC_RDY = 2,
    AVSDEC_HDRS_RDY = 3,
    AVSDEC_HDRS_NOT_RDY = 4,
    AVSDEC_PIC_DECODED = 5,
    AVSDEC_NONREF_PIC_SKIPPED = 6,/* Skipped non-reference picture */
#ifdef USE_OUTPUT_RELEASE
    AVSDEC_ABORTED = 7,
    AVSDEC_END_OF_STREAM = 8,
#endif

#ifdef USE_EXTERNAL_BUFFER
    /** Waiting for external buffers allocated. */
    AVSDEC_WAITING_FOR_BUFFER = 9,
#endif
#ifdef USE_OUTPUT_RELEASE
    AVSDEC_FLUSHED = 10,
#endif
    AVSDEC_BUF_EMPTY = 11,
    AVSDEC_PARAM_ERROR = -1,
    AVSDEC_STRM_ERROR = -2,
    AVSDEC_NOT_INITIALIZED = -3,
    AVSDEC_MEMFAIL = -4,
    AVSDEC_INITFAIL = -5,
    AVSDEC_STREAM_NOT_SUPPORTED = -8,
#ifdef USE_EXTERNAL_BUFFER
    AVSDEC_EXT_BUFFER_REJECTED = -9, /**<\hideinitializer */
#endif

    AVSDEC_HW_RESERVED = -254,
    AVSDEC_HW_TIMEOUT = -255,
    AVSDEC_HW_BUS_ERROR = -256,
    AVSDEC_SYSTEM_ERROR = -257,
    AVSDEC_DWL_ERROR = -258,
    AVSDEC_FORMAT_NOT_SUPPORTED = -1000
} AvsDecRet;

/* decoder output picture format */
typedef enum
{
    AVSDEC_SEMIPLANAR_YUV420 = 0x020001,
    AVSDEC_TILED_YUV420 = 0x020002
} AvsDecOutFormat;

/* DAR (Display aspect ratio) */
typedef enum
{
    AVSDEC_1_1 = 0x01,
    AVSDEC_4_3 = 0x02,
    AVSDEC_16_9 = 0x03,
    AVSDEC_2_21_1 = 0x04
} AvsDecDARFormat;

/* SAR (Sample aspect ratio) */
/* TODO! */

typedef struct
{
    u32 *pVirtualAddress;
    g1_addr_t busAddress;
} AvsDecLinearMem;

/* Decoder instance */
typedef void *AvsDecInst;

/* Input structure */
typedef struct
{
    u8 *pStream;         /* Pointer to stream to be decoded              */
    g1_addr_t streamBusAddress;   /* DMA bus address of the input stream */
    u32 dataLen;         /* Number of bytes to be decoded                */
    u32 picId;
    u32 skipNonReference; /* Flag to enable decoder skip non-reference
                               * frames to reduce processor load */
} AvsDecInput;

/* Time code */
typedef struct
{
    u32 hours;
    u32 minutes;
    u32 seconds;
    u32 pictures;
} AvsDecTime;

typedef struct
{
    u8 *pStrmCurrPos;
    g1_addr_t strmCurrBusAddress; /* DMA bus address location where the decoding
                                 * ended */
    u32 dataLeft;
} AvsDecOutput;

/* stream info filled by AvsDecGetInfo */
typedef struct
{
    u32 frameWidth;
    u32 frameHeight;
    u32 codedWidth;
    u32 codedHeight;
    u32 profileId;
    u32 levelId;
    u32 displayAspectRatio;
    u32 videoFormat;
    u32 videoRange;
    u32 interlacedSequence;
    DecDpbMode dpbMode;         /* DPB mode; frame, or field interlaced */
#ifdef USE_EXTERNAL_BUFFER
    u32 picBuffSize;
#endif
    u32 multiBuffPpSize;
    AvsDecOutFormat outputFormat;
} AvsDecInfo;

typedef struct
{
    u8 *pOutputPicture;
    g1_addr_t outputPictureBusAddress;
    u32 frameWidth;
    u32 frameHeight;
    u32 codedWidth;
    u32 codedHeight;
    u32 keyPicture;
    u32 picId;
    u32 decodeId;
    u32 picCodingType;
    u32 interlaced;
    u32 fieldPicture;
    u32 topField;
    u32 firstField;
    u32 repeatFirstField;
    u32 repeatFrameCount;
    u32 numberOfErrMBs;
    DecOutFrmFormat outputFormat;
    AvsDecTime timeCode;
} AvsDecPicture;

/* Version information */
typedef struct
{
    u32 major;           /* API major version */
    u32 minor;           /* API minor version */

} AvsDecApiVersion;

typedef struct DecSwHwBuild_  AvsDecBuild;
#ifdef USE_EXTERNAL_BUFFER
/*!\struct AvsDecBufferInfo_
 * \brief Reference buffer information
 *
 * A structure containing the reference buffer information, filled by
 * AvsDecGetBufferInfo()
 *
 * \typedef AvsDecBufferInfo
 * A typename for #AvsDecBufferInfo_.
 */
typedef struct AvsDecBufferInfo_
{
    u32 nextBufSize;
    u32 bufNum;
    DWLLinearMem_t bufToFree;
#ifdef ASIC_TRACE_SUPPORT
    u32 is_frame_buffer;
#endif
} AvsDecBufferInfo;
#endif

/*------------------------------------------------------------------------------
    Prototypes of Decoder API functions
------------------------------------------------------------------------------*/

AvsDecApiVersion AvsDecGetAPIVersion(void);

AvsDecBuild AvsDecGetBuild(void);

AvsDecRet AvsDecInit(AvsDecInst *pDecInst,
#ifdef USE_EXTERNAL_BUFFER
                     const void *dwl,
#endif
                     DecErrorHandling errorHandling,
                     u32 numFrameBuffers,
                     DecDpbFlags dpbFlags,
                     u32 useAdaptiveBuffers,
                     u32 nGuardSize);

AvsDecRet AvsDecDecode(AvsDecInst decInst,
                       AvsDecInput *pInput,
                       AvsDecOutput *pOutput);

AvsDecRet AvsDecGetInfo(AvsDecInst decInst, AvsDecInfo *pDecInfo);

AvsDecRet AvsDecNextPicture(AvsDecInst decInst,
                            AvsDecPicture *pPicture,
                            u32 endOfStream);

#ifdef USE_OUTPUT_RELEASE
AvsDecRet AvsDecPictureConsumed(AvsDecInst decInst,
                                AvsDecPicture *pPicture);

AvsDecRet AvsDecEndOfStream(AvsDecInst decInst, u32 strmEndFlag);
#endif

void AvsDecRelease(AvsDecInst decInst);

AvsDecRet AvsDecPeek(AvsDecInst decInst, AvsDecPicture *pPicture);
#ifdef USE_EXTERNAL_BUFFER
AvsDecRet AvsDecGetBufferInfo(AvsDecInst decInst, AvsDecBufferInfo *mem_info);

AvsDecRet AvsDecAddBuffer(AvsDecInst decInst, DWLLinearMem_t *info);
#endif
#ifdef USE_OUTPUT_RELEASE
AvsDecRet AvsDecAbort(AvsDecInst decInst);

AvsDecRet AvsDecAbortAfter(AvsDecInst decInst);
#endif

/*------------------------------------------------------------------------------
    Prototype of the API trace funtion. Traces all API entries and returns.
    This must be implemented by the application using the decoder API!
    Argument:
        string - trace message, a null terminated string
------------------------------------------------------------------------------*/
void AvsDecTrace(const char *string);

#ifdef __cplusplus
}
#endif

#endif                       /* __AVSDECAPI_H__ */
