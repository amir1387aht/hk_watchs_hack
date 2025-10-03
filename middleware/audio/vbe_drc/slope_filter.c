/*************************************************************
Slope Filter Source Code
File: slope_filter.c
*************************************************************/
#include <math.h>
#include "slope_filter.h"

void slope_lpf(float *x, int frame_length, float *state, const float *FB, const float *FA)
{
    float s1, s2, s3;
    float y;
    int n;

    for (n = 0; n < frame_length; n++)
    {
        // first stage
        s1 = FB[0] * x[n] + FB[1] * state[0] + FB[2] * state[1] + FA[0] * state[2] + FA[1] * state[3];
        // second stage
        s2 = FB[3] * s1 + FB[4] * state[4] + FB[5] * state[5] + FA[2] * state[6] + FA[3] * state[7];
        // third stage
        s3 = FB[6] * s2 + FB[7] * state[8] + FB[8] * state[9] + FA[4] * state[10] + FA[5] * state[11];
        // fourth stage
        y = FB[9] * s3 + FB[10] * state[12] + FB[11] * state[13] + FA[6] * state[14] + FA[7] * state[15];

        //update state
        state[1] = state[0];
        state[0] = x[n];
        state[3] = state[2];
        state[2] = s1;

        state[5] = state[4];
        state[4] = s1;
        state[7] = state[6];
        state[6] = s2;

        state[9] = state[8];
        state[8] = s2;
        state[11] = state[10];
        state[10] = s3;

        state[13] = state[12];
        state[12] = s3;
        state[15] = state[14];
        state[14] = y;

        x[n] = y;
    }
}

void slope_hpf(float *x, int frame_length, float *state, const float *FB, const float *FA)
{
    float s1;
    float y;
    int n;

    for (n = 0; n < frame_length; n++)
    {
        // first stage
        s1 = FB[0] * x[n] + FB[1] * state[0] + FB[2] * state[1] + FA[0] * state[2] + FA[1] * state[3];
        // second stage
        y = FB[3] * s1 + FB[4] * state[4] + FB[5] * state[5] + FA[2] * state[6] + FA[3] * state[7];

        //update state
        state[1] = state[0];
        state[0] = x[n];
        state[3] = state[2];
        state[2] = s1;

        state[5] = state[4];
        state[4] = s1;
        state[7] = state[6];
        state[6] = y;

        x[n] = y;
    }
}
