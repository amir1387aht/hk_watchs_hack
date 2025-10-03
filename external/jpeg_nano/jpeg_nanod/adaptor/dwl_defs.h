/* Copyright 2012 Google Inc. All Rights Reserved. */
/* Author: attilanagy@google.com (Atti Nagy) */

#ifndef SOFTWARE_LINUX_DWL_DWL_DEFS_H_
#define SOFTWARE_LINUX_DWL_DWL_DEFS_H_

#define DWL_MPEG2_E         31  /* 1 bit */
#define DWL_VC1_E           29  /* 2 bits */
#define DWL_JPEG_E          28  /* 1 bit */
#define DWL_MPEG4_E         26  /* 2 bits */
#define DWL_H264_E          24  /* 2 bits */
#define DWL_VP6_E           23  /* 1 bit */
#define DWL_RV_E            26  /* 2 bits */
#define DWL_VP8_E           23  /* 1 bit */
#define DWL_VP7_E           24  /* 1 bit */
#define DWL_WEBP_E          19  /* 1 bit */
#define DWL_AVS_E           22  /* 1 bit */
#define DWL_PP_E            16  /* 1 bit */

#define HX170_IRQ_STAT_DEC          1
#define HX170_IRQ_STAT_DEC_OFF      (HX170_IRQ_STAT_DEC * 4)
#define HX170_IRQ_STAT_PP           60
#define HX170_IRQ_STAT_PP_OFF       (HX170_IRQ_STAT_PP * 4)

#define HX170PP_SYNTH_CFG           100
#define HX170PP_SYNTH_CFG_OFF       (HX170PP_SYNTH_CFG * 4)
#define HX170DEC_SYNTH_CFG          50
#define HX170DEC_SYNTH_CFG_OFF      (HX170DEC_SYNTH_CFG * 4)
#define HX170DEC_SYNTH_CFG_2        54
#define HX170DEC_SYNTH_CFG_2_OFF    (HX170DEC_SYNTH_CFG_2 * 4)


#define HX170_DEC_E                 0x01
#define HX170_PP_E                  0x01
#define HX170_DEC_ABORT             0x20
#define HX170_DEC_IRQ_DISABLE       0x10
#define HX170_PP_IRQ_DISABLE        0x10
#define HX170_DEC_IRQ               0x100
#define HX170_PP_IRQ                0x100

#endif /* SOFTWARE_LINUX_DWL_DWL_DEFS_H_ */
