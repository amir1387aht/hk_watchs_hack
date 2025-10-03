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
--  Description : JPEG decoder header file
--
------------------------------------------------------------------------------*/
#ifndef __JPEGDECAPI_EX_H__
#define __JPEGDECAPI_EX_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "jpegdecapi.h"

JpegDecRet JpegDecInitReg(JpegDecInst decInst);

JpegDecRet JpegDecUpdateSliceMbSet(JpegDecInst decInst, JpegDecInput *pDecIn,
                                JpegDecImageInfo *pImageInfo, u32 amountOfMCUs,
                                u32 mcuInRow, u32 mcuSizeDivider);

const void *JpegDecGetDwlInst(JpegDecInst decInst);

#ifdef __cplusplus
}
#endif

#endif
