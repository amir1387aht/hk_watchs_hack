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
--  Description : API for the 8190 RV Decoder
--
------------------------------------------------------------------------------*/

#ifndef __RVDECAPI_H__
#define __RVDECAPI_H__

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
    RVDEC_OK = 0,
    RVDEC_STRM_PROCESSED = 1,
    RVDEC_PIC_RDY = 2,
    RVDEC_HDRS_RDY = 3,
    RVDEC_HDRS_NOT_RDY = 4,
    RVDEC_PIC_DECODED = 5,
    RVDEC_NONREF_PIC_SKIPPED = 6,/* Skipped non-reference picture */
#ifdef USE_OUTPUT_RELEASE
    RVDEC_ABORTED = 7,
    RVDEC_END_OF_STREAM = 8,
#endif
#ifdef USE_EXTERNAL_BUFFER
    /** Waiting for external buffers allocated. */
    RVDEC_WAITING_FOR_BUFFER = 9,
#endif
#ifdef USE_OUTPUT_RELEASE
    RVDEC_FLUSHED = 10,
#endif

    RVDEC_BUF_EMPTY = 11,
    RVDEC_PARAM_ERROR = -1,
    RVDEC_STRM_ERROR = -2,
    RVDEC_NOT_INITIALIZED = -3,
    RVDEC_MEMFAIL = -4,
    RVDEC_INITFAIL = -5,
    RVDEC_STREAM_NOT_SUPPORTED = -8,
#ifdef USE_EXTERNAL_BUFFER
    RVDEC_EXT_BUFFER_REJECTED = -9, /**<\hideinitializer */
#endif

    RVDEC_HW_RESERVED = -254,
    RVDEC_HW_TIMEOUT = -255,
    RVDEC_HW_BUS_ERROR = -256,
    RVDEC_SYSTEM_ERROR = -257,
    RVDEC_DWL_ERROR = -258,
    RVDEC_FORMAT_NOT_SUPPORTED = -1000
} RvDecRet;

/* decoder output picture format */
typedef enum
{
    RVDEC_SEMIPLANAR_YUV420 = 0x020001,
    RVDEC_TILED_YUV420 = 0x020002
} RvDecOutFormat;

typedef struct
{
    u32 *pVirtualAddress;
    g1_addr_t busAddress;
} RvDecLinearMem;

/* Decoder instance */
typedef void *RvDecInst;

typedef struct
{
    u32 offset;
    u32 isValid;
} RvDecSliceInfo;

/* Input structure */
typedef struct
{
    u8 *pStream;             /* Pointer to stream to be decoded              */
    g1_addr_t streamBusAddress;    /* DMA bus address of the input stream */
    u32 dataLen;             /* Number of bytes to be decoded                */
    u32 picId;
    u32 timestamp;       /* timestamp of current picture from rv frame header.
                              * NOTE: timestamp of a B-frame should be adjusted referring
                              * to its forward reference frame's timestamp */

    u32 sliceInfoNum;    /* The number of slice offset entries. */
    RvDecSliceInfo *pSliceInfo;     /* Pointer to the sliceInfo.
                                         * It contains offset value of each slice
                                         * in the data buffer, including start point "0"
                                         * and end point "dataLen" */
    u32 skipNonReference; /* Flag to enable decoder skip non-reference
                               * frames to reduce processor load */
} RvDecInput;

typedef struct
{
    u8 *pStrmCurrPos;
    g1_addr_t strmCurrBusAddress;          /* DMA bus address location where the decoding
                                          * ended */
    u32 dataLeft;
} RvDecOutput;

/* stream info filled by RvDecGetInfo */
typedef struct
{
    u32 frameWidth;
    u32 frameHeight;
    u32 codedWidth;
    u32 codedHeight;
    u32 multiBuffPpSize;
#ifdef USE_EXTERNAL_BUFFER
    u32 picBuffSize;
#endif
    DecDpbMode dpbMode;         /* DPB mode; frame, or field interlaced */
    RvDecOutFormat outputFormat;
} RvDecInfo;

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
    u32 numberOfErrMBs;
    DecOutFrmFormat outputFormat;
} RvDecPicture;

/* Version information */
typedef struct
{
    u32 major;           /* API major version */
    u32 minor;           /* API minor version */

} RvDecApiVersion;

typedef struct DecSwHwBuild_  RvDecBuild;

#ifdef USE_EXTERNAL_BUFFER
/*!\struct RvDecBufferInfo_
 * \brief Reference buffer information
 *
 * A structure containing the reference buffer information, filled by
 * RvDecGetBufferInfo()
 *
 * \typedef RvDecBufferInfo
 * A typename for #RvDecBufferInfo_.
 */
typedef struct RvDecBufferInfo_
{
    u32 nextBufSize;
    u32 bufNum;
    DWLLinearMem_t bufToFree;
#ifdef ASIC_TRACE_SUPPORT
    u32 is_frame_buffer;
#endif
} RvDecBufferInfo;
#endif

/*------------------------------------------------------------------------------
    Prototypes of Decoder API functions
------------------------------------------------------------------------------*/

RvDecApiVersion RvDecGetAPIVersion(void);

RvDecBuild RvDecGetBuild(void);

RvDecRet RvDecInit(RvDecInst *pDecInst,
#ifdef USE_EXTERNAL_BUFFER
                   const void *dwl,
#endif
                   DecErrorHandling errorHandling,
                   u32 frameCodeLength,
                   u32 *frameSizes,
                   u32 rvVersion,
                   u32 maxFrameWidth, u32 maxFrameHeight,
                   u32 numFrameBuffers,
                   DecDpbFlags dpbFlags,
                   u32 useAdaptiveBuffers,
                   u32 nGuardSize);

RvDecRet RvDecDecode(RvDecInst decInst,
                     RvDecInput *pInput,
                     RvDecOutput *pOutput);

RvDecRet RvDecGetInfo(RvDecInst decInst, RvDecInfo *pDecInfo);

RvDecRet RvDecNextPicture(RvDecInst decInst,
                          RvDecPicture *pPicture,
                          u32 endOfStream);
#ifdef USE_OUTPUT_RELEASE
RvDecRet RvDecPictureConsumed(RvDecInst decInst,
                              RvDecPicture *pPicture);

RvDecRet RvDecEndOfStream(RvDecInst decInst, u32 strmEndFlag);
#endif

void RvDecRelease(RvDecInst decInst);

RvDecRet RvDecPeek(RvDecInst decInst, RvDecPicture *pPicture);
#ifdef USE_EXTERNAL_BUFFER
RvDecRet RvDecGetBufferInfo(RvDecInst decInst, RvDecBufferInfo *mem_info);

RvDecRet RvDecAddBuffer(RvDecInst decInst, DWLLinearMem_t *info);
#endif
#ifdef USE_OUTPUT_RELEASE
RvDecRet RvDecAbort(RvDecInst decInst);

RvDecRet RvDecAbortAfter(RvDecInst decInst);
#endif

/*------------------------------------------------------------------------------
    Prototype of the API trace funtion. Traces all API entries and returns.
    This must be implemented by the application using the decoder API!
    Argument:
        string - trace message, a null terminated string
------------------------------------------------------------------------------*/
void RvDecTrace(const char *string);

#ifdef __cplusplus
}
#endif

#endif                       /* __RVDECAPI_H__ */
