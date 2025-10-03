#ifndef VBE_DRC_EQ_H
#define VBE_DRC_EQ_H 1
#include "rtconfig.h"
#include "vbe_func.h"
#include "drc_func.h"
#include "crossover_filter.h"
#include "slope_filter.h"
#include "vb_filter_coef.h"

typedef struct
{
    void        *callback_user_data;
    //drc
    float drc_gs_low_left;
    float drc_gs_mid_left;
    float drc_gs_hi_left;
    float drc_gs_low_right;
    float drc_gs_mid_right;
    float drc_gs_hi_right;

    DRC_Para    drcParaLow;
    DRC_Para    drcParaMid;
    DRC_Para    drcParaHi;
    /**************************************************************/
    // DRC Crossover
    //                      |LP2--
    //           ----LP1----
    //          |           |HP2--
    //       ---
    //          |
    //           ----HP1----(LP2 + HP2)--
    /*************************************************************/
    float crossover_lpf1_out[VBE_ONE_FRAME_SAMPLES];
    float crossover_hpf1_out[VBE_ONE_FRAME_SAMPLES];
    float crossover_lpf2d_out[VBE_ONE_FRAME_SAMPLES];
    float crossover_hpf2d_out[VBE_ONE_FRAME_SAMPLES];

    float crossover_low_left[VBE_ONE_FRAME_SAMPLES];
    float crossover_mid_left[VBE_ONE_FRAME_SAMPLES];
    float crossover_hi_left[VBE_ONE_FRAME_SAMPLES];
    float crossover_low_right[VBE_ONE_FRAME_SAMPLES];
    float crossover_mid_right[VBE_ONE_FRAME_SAMPLES];
    float crossover_hi_right[VBE_ONE_FRAME_SAMPLES];
    float drc_low_left_out[VBE_ONE_FRAME_SAMPLES];
    float drc_mid_left_out[VBE_ONE_FRAME_SAMPLES];
    float drc_hi_left_out[VBE_ONE_FRAME_SAMPLES];
    float drc_low_right_out[VBE_ONE_FRAME_SAMPLES];
    float drc_mid_right_out[VBE_ONE_FRAME_SAMPLES];
    float drc_hi_right_out[VBE_ONE_FRAME_SAMPLES];
    float drc_left_out[VBE_ONE_FRAME_SAMPLES];
    float drc_right_out[VBE_ONE_FRAME_SAMPLES];
#if BSP_USING_CMSIS_DSP
    float dsp_coeffs_crossover_lpf[3 + 2 + 3 + 2];
    float dsp_coeffs_crossover_hpf[3 + 2 + 3 + 2];
    float dsp_coeffs_crossover_lpf1[(6 + 4) * 2];
    float dsp_coeffs_crossover_lpf2[(6 + 4) * 2];
    float dsp_coeffs_crossover_hpf1[(6 + 4) * 2];
    float dsp_coeffs_crossover_hpf2[(6 + 4) * 2];
    float dsp_coeffs_slope_lpf[12 + 8];
    float dsp_coeffs_slope_hpf[6 + 4];
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf1_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf1_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf2u_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf2u_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf2d_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lpf2d_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf1_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf1_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf2u_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf2u_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf2d_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_hpf2d_right;
    arm_biquad_casd_df1_inst_f32 dsp_inst_crossover_lfp_left;
    arm_biquad_casd_df1_inst_f32 dsp_inst_slope_lpf;
    arm_biquad_casd_df1_inst_f32 dsp_inst_slope_hpf;
#endif
    float crossover_lpf1_state_left[16];
    float crossover_hpf1_state_left[16];
    float crossover_lpf2u_state_left[16];
    float crossover_hpf2u_state_left[16];
    float crossover_lpf2d_state_left[16];
    float crossover_hpf2d_state_left[16];

    float crossover_lpf1_state_right[16];
    float crossover_hpf1_state_right[16];
    float crossover_lpf2u_state_right[16];
    float crossover_hpf2u_state_right[16];
    float crossover_lpf2d_state_right[16];
    float crossover_hpf2d_state_right[16];
    //vbe
    float       crossover_lpf_state_left[8];
    float       crossover_lpf_state_right[8];
    float       crossover_hpf_state_left[8];
    float       crossover_hpf_state_right[8];
    float       slope_lpf_state[16];
    float       slope_hpf_state[8];
    float       x_lp_left[VBE_ONE_FRAME_SAMPLES];
    float       x_lp_right[VBE_ONE_FRAME_SAMPLES];
    float       x_hp_left[VBE_ONE_FRAME_SAMPLES];
    float       x_hp_right[VBE_ONE_FRAME_SAMPLES];
    float       audio_data_left[VBE_ONE_FRAME_SAMPLES];
    float       audio_data_right[VBE_ONE_FRAME_SAMPLES];
    float       x_mono[VBE_ONE_FRAME_SAMPLES];
#if BSP_USING_CMSIS_DSP
    float       x_mono_out[VBE_ONE_FRAME_SAMPLES];
#endif
    //eq
    float eq_coef_f[32 * 5];
    float eq_state_left[64];
    float eq_state_right[64];
    float eq_out_left[VBE_ONE_FRAME_SAMPLES];
    float eq_out_right[VBE_ONE_FRAME_SAMPLES];

    uint16_t    remain_samples;
    uint8_t     channels;
    uint8_t     bit_width;
    uint8_t     should_update_config;
    uint8_t     drc_enable;
} vbe_drc_t;

#endif
