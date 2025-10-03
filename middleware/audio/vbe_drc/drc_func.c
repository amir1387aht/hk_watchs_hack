/*************************************************************
Audio Dynamic Range Control Source Code
File: drc_func.c
*************************************************************/
#include <math.h>
#include "drc_func.h"

void drc_low_left(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float g, gs;
    float alphaA, alphaR, betaA, betaR;
    float x, x_abs, x_db;

    int n;

    gs = (*drc_gs);
    alphaA = p_drc_para->alphaA;
    betaA = p_drc_para->betaA;
    alphaR = p_drc_para->alphaR;
    betaR = p_drc_para->betaR;

    for (n = 0; n < frame_length; n++)
    {
        // cover input to dB
        x = data_in[n];

        if ((x < 1) && (x > -1))
            x = 1;

        x = x / 8388608.0f;

        if (x < 0)
            x_abs = (-1) * x;
        else
            x_abs = x;

        x_db = 20.0f * fast_log10(x_abs);

        if (x_db <= (p_drc_para->expanderThreshold + 0.5f * p_drc_para->expanderKneeWidth))
            g = expander_gain(drc_para_inst, x_db);
        else if (x_db >= (p_drc_para->compressorThreshold - 0.5f * p_drc_para->compressorKneeWidth))
            g = compressor_gain(drc_para_inst, x_db);
        else
            g = 0;

        // gain smoothing
        if (g <= gs)
            gs = alphaA * gs + betaA * g;
        else
            gs = alphaR * gs + betaR * g;

        gs += p_drc_para->makeupGain;

        // apply gain
        data_out[n] = apply_gain(data_in[n], gs);
    }

    // update gain state
    *drc_gs = gs;
}

void drc_mid_left(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float g, gs;
    float alphaA, alphaR, betaA, betaR;
    float x, x_abs, x_db;

    int n;

    gs = (*drc_gs);
    alphaA = p_drc_para->alphaA;
    betaA = p_drc_para->betaA;
    alphaR = p_drc_para->alphaR;
    betaR = p_drc_para->betaR;

    for (n = 0; n < frame_length; n++)
    {
        // cover input to dB
        x = data_in[n];

        if ((x < 1) && (x > -1))
            x = 1;

        x = x / 8388608.0f;

        if (x < 0)
            x_abs = (-1) * x;
        else
            x_abs = x;

        x_db = 20.0f * fast_log10(x_abs);

        if (x_db <= (p_drc_para->expanderThreshold + 0.5f * p_drc_para->expanderKneeWidth))
            g = expander_gain(drc_para_inst, x_db);
        else if (x_db >= (p_drc_para->compressorThreshold - 0.5f * p_drc_para->compressorKneeWidth))
            g = compressor_gain(drc_para_inst, x_db);
        else
            g = 0;

        // gain smoothing
        if (g <= gs)
            gs = alphaA * gs + betaA * g;
        else
            gs = alphaR * gs + betaR * g;

        gs += p_drc_para->makeupGain;

        // apply gain
        data_out[n] = apply_gain(data_in[n], gs);
    }

    // update gain state
    *drc_gs = gs;
}

void drc_hi_left(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float g, gs;
    float alphaA, alphaR, betaA, betaR;
    float x, x_abs, x_db;

    int n;

    gs = (*drc_gs);
    alphaA = p_drc_para->alphaA;
    betaA = p_drc_para->betaA;
    alphaR = p_drc_para->alphaR;
    betaR = p_drc_para->betaR;

    for (n = 0; n < frame_length; n++)
    {
        // cover input to dB
        x = data_in[n];

        if ((x < 1) && (x > -1))
            x = 1;

        x = x / 8388608.0f;

        if (x < 0)
            x_abs = (-1) * x;
        else
            x_abs = x;

        x_db = 20.0f * fast_log10(x_abs);

        if (x_db <= (p_drc_para->expanderThreshold + 0.5f * p_drc_para->expanderKneeWidth))
            g = expander_gain(drc_para_inst, x_db);
        else if (x_db >= (p_drc_para->compressorThreshold - 0.5f * p_drc_para->compressorKneeWidth))
            g = compressor_gain(drc_para_inst, x_db);
        else
            g = 0;

        // gain smoothing
        if (g <= gs)
            gs = alphaA * gs + betaA * g;
        else
            gs = alphaR * gs + betaR * g;

        gs += p_drc_para->makeupGain;

        // apply gain
        data_out[n] = apply_gain(data_in[n], gs);
    }

    // update gain state
    *drc_gs = gs;
}

void drc_low_right(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float g, gs;
    float alphaA, alphaR, betaA, betaR;
    float x, x_abs, x_db;

    int n;

    gs = (*drc_gs);
    alphaA = p_drc_para->alphaA;
    betaA = p_drc_para->betaA;
    alphaR = p_drc_para->alphaR;
    betaR = p_drc_para->betaR;

    for (n = 0; n < frame_length; n++)
    {
        // cover input to dB
        x = data_in[n];

        if ((x < 1) && (x > -1))
            x = 1;

        x = x / 8388608.0f;

        if (x < 0)
            x_abs = (-1) * x;
        else
            x_abs = x;

        x_db = 20.0f * fast_log10(x_abs);

        if (x_db <= (p_drc_para->expanderThreshold + 0.5f * p_drc_para->expanderKneeWidth))
            g = expander_gain(drc_para_inst, x_db);
        else if (x_db >= (p_drc_para->compressorThreshold - 0.5f * p_drc_para->compressorKneeWidth))
            g = compressor_gain(drc_para_inst, x_db);
        else
            g = 0;

        // gain smoothing
        if (g <= gs)
            gs = alphaA * gs + betaA * g;
        else
            gs = alphaR * gs + betaR * g;

        gs += p_drc_para->makeupGain;

        // apply gain
        data_out[n] = apply_gain(data_in[n], gs);
    }

    // update gain state
    *drc_gs = gs;
}

void drc_mid_right(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float g, gs;
    float alphaA, alphaR, betaA, betaR;
    float x, x_abs, x_db;

    int n;

    gs = (*drc_gs);
    alphaA = p_drc_para->alphaA;
    betaA = p_drc_para->betaA;
    alphaR = p_drc_para->alphaR;
    betaR = p_drc_para->betaR;

    for (n = 0; n < frame_length; n++)
    {
        // cover input to dB
        x = data_in[n];

        if ((x < 1) && (x > -1))
            x = 1;

        x = x / 8388608.0f;

        if (x < 0)
            x_abs = (-1) * x;
        else
            x_abs = x;

        x_db = 20.0f * fast_log10(x_abs);

        if (x_db <= (p_drc_para->expanderThreshold + 0.5f * p_drc_para->expanderKneeWidth))
            g = expander_gain(drc_para_inst, x_db);
        else if (x_db >= (p_drc_para->compressorThreshold - 0.5f * p_drc_para->compressorKneeWidth))
            g = compressor_gain(drc_para_inst, x_db);
        else
            g = 0;

        // gain smoothing
        if (g <= gs)
            gs = alphaA * gs + betaA * g;
        else
            gs = alphaR * gs + betaR * g;

        gs += p_drc_para->makeupGain;

        // apply gain
        data_out[n] = apply_gain(data_in[n], gs);
    }

    // update gain state
    *drc_gs = gs;
}

void drc_hi_right(float *data_out, float *data_in, void *drc_para_inst, float *drc_gs, int frame_length)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float g, gs;
    float alphaA, alphaR, betaA, betaR;
    float x, x_abs, x_db;

    int n;

    gs = (*drc_gs);
    alphaA = p_drc_para->alphaA;
    betaA = p_drc_para->betaA;
    alphaR = p_drc_para->alphaR;
    betaR = p_drc_para->betaR;

    for (n = 0; n < frame_length; n++)
    {
        // cover input to dB
        x = data_in[n];

        if ((x < 1) && (x > -1))
            x = 1;

        x = x / 8388608.0f;

        if (x < 0)
            x_abs = (-1) * x;
        else
            x_abs = x;

        x_db = 20.0f * fast_log10(x_abs);

        if (x_db <= (p_drc_para->expanderThreshold + 0.5f * p_drc_para->expanderKneeWidth))
            g = expander_gain(drc_para_inst, x_db);
        else if (x_db >= (p_drc_para->compressorThreshold - 0.5f * p_drc_para->compressorKneeWidth))
            g = compressor_gain(drc_para_inst, x_db);
        else
            g = 0;

        // gain smoothing
        if (g <= gs)
            gs = alphaA * gs + betaA * g;
        else
            gs = alphaR * gs + betaR * g;

        gs += p_drc_para->makeupGain;

        // apply gain
        data_out[n] = apply_gain(data_in[n], gs);
    }

    // update gain state
    *drc_gs = gs;
}


float expander_gain(void *drc_para_inst, float x_db)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float T, R, W;
    T = p_drc_para->expanderThreshold;
    R = p_drc_para->expanderRatio;
    W = p_drc_para->expanderKneeWidth;

    float y_db;
    float xr;
    float g;

    xr = 2 * (x_db - T);

    if (xr < -W)
    {
        y_db = T + (x_db - T) * R;
    }
    else if (xr >= W)
    {
        y_db = x_db;
    }
    else
    {
        y_db = x_db + (1 - R) * (x_db - T - 0.5f * W) * (x_db - T - 0.5f * W) / (2.0f * W);
    }

    g = y_db - x_db;

    return g;
}

float compressor_gain(void *drc_para_inst, float x_db)
{
    DRC_Para *p_drc_para = drc_para_inst;
    float T, R, W;
    T = p_drc_para->compressorThreshold;
    R = p_drc_para->compressorRatio;
    W = p_drc_para->compressorKneeWidth;

    float y_db;
    float xr;
    float g;

    xr = 2 * (x_db - T);

    if (xr > W)
    {
        if (R > 0)
            y_db = T + (x_db - T) / R;
        else
            y_db = T;
    }
    else if (xr <= -W)
    {
        y_db = x_db;
    }
    else
    {

        if (R > 0)
            y_db = x_db + (1.0f / R - 1.0f) * (x_db - T + 0.5f * W) * (x_db - T + 0.5f * W) / (2.0f * W);
        else
            y_db = x_db - (x_db - T + 0.5f * W) * (x_db - T + 0.5f * W) / (2.0f * W);
    }

    g = y_db - x_db;

    return g;
}

float apply_gain(float x, float g)
{
    int g0, g1, num_shift, num_shift_a;
    float res_db, res0, res1, res2, res_g;
    int db_index;
    float gain;
    float y;

    g0 = (int) floor(g);
    num_shift = g0 / 6;
    g1 = g0 - num_shift * 6;
    if (g1 < 0)
    {
        num_shift--;
    }
    res_db = g - num_shift * 6.0f;
    db_index = (int) floor(res_db * 2.0f);
    res_db = res_db - db_index * 0.5f;
    num_shift_a = -num_shift;

    switch (db_index)
    {
    case 0:  // 0dB
        gain = 1.0f;
        break;
    case 1:  // 0.5dB
        gain = 1.059253725177289f;
        break;
    case 2:  // 1dB
        gain = 1.122018454301963f;
        break;
    case 3:  // 1.5dB
        gain = 1.188502227437018f;
        break;
    case 4:  // 2dB
        gain = 1.258925411794167f;
        break;
    case 5:  // 2.5dB
        gain = 1.333521432163324f;
        break;
    case 6:  // 3dB
        gain = 1.412537544622754f;
        break;
    case 7:  // 3.5dB
        gain = 1.496235656094433f;
        break;
    case 8:  // 4dB
        gain = 1.584893192461114f;
        break;
    case 9:  // 4.5dB
        gain = 1.678804018122560f;
        break;
    case 10:  // 5dB
        gain = 1.778279410038923f;
        break;
    case 11:  // 5.5dB
        gain = 1.883649089489801f;
        break;
    case 12:  // 5.5dB
        gain = 2.0f;
        break;
    default:
        gain = 1.0f;
        break;
    }

    //y = x * pow(2, -num_shift_a);
    if (num_shift_a >= 0)
        y = x / (1 << num_shift_a);
    else
        y = x * (1 << ((-1) * num_shift_a));

    y = y * gain;

    res0 = 0.115129254649702f * res_db;
    res1 = res0 * res0;
    res2 = res0 * res1;
    res_g = 1.0f + res0 + 0.5f * res1 + res2 * 0.16666666666f;

    y = y * res_g;

    return y;
}

