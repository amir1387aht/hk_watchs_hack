#ifndef _OSAL_SIFLI_H_
#define _OSAL_SIFLI_H_

#ifdef __cplusplus
extern "C" {
#endif



#include "os_adaptor.h"

#ifdef USING_JPEG_NANODEC
#define PP_JPEGDEC_PIPELINE_SUPPORT
#define PP_PIPELINE_ENABLED



//#define JPEGDEC_TRACE
//#define PP_TRACE

#define PP_X170_DATA_BUS_WIDTH PP_X170_DATA_BUS_WIDTH_32

//#define _PPDEBUG_PRINT

#define PP_X170_OUTPUT_PICTURE_ENDIAN_RGB16 PP_X170_PICTURE_BIG_ENDIAN
#define PP_X170_SWAP_16_WORDS_RGB16  1

#define _ASSERT_USED
#endif /* USING_JPEG_NANODEC */


#ifdef __cplusplus
}
#endif

#endif /* _OSAL_SIFLI_H_ */
