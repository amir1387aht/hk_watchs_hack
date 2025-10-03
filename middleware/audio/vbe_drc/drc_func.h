/********************************************************
File: DRC_FUNC.h
********************************************************/
#ifndef DRC_FUNC_H
#define DRC_FUNC_H


#include <stdint.h>
#include "fast_log10.h"


#ifdef __cplusplus
extern "C" {
#endif

/*audio DRC parameters */
typedef struct
{
    int enable;
    float compressorThreshold;  // -30~0  dB
    float compressorRatio;      // 0~10
    float compressorKneeWidth;  // 0~10  dB
    float expanderThreshold;    // -120~-60  dB
    float expanderRatio;        // 0~10
    float expanderKneeWidth;    // 0~10  dB
    float alphaA;
    float betaA;
    float alphaR;
    float betaR;
    int makeupGain;
} DRC_Para;

void drc_low_left(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length);
void drc_mid_left(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length);
void drc_hi_left(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length);
void drc_low_right(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length);
void drc_mid_right(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length);
void drc_hi_right(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length);

float expander_gain(void *drc_para_inst, float x_db);
float compressor_gain(void *drc_para_inst, float x_db);
float apply_gain(float x, float g);

#ifdef __cplusplus
}
#endif
#endif
/* DRC_FUNC_H */
