/*************************************************************
Crossover Filter Source Code
File: crossover_filter.c
*************************************************************/
#include <math.h>
#include "crossover_filter.h"

// biquid filter
// y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-1] + a1 * y[n-1] + a2* y[n-2]

void crossover_lpf_left(float *data_out, float *x, int frame_length, float *state, const float *FB, const float *FA)
{
    float s1;
    float y;
    int n;
    int d = 0;

    for (n = 0; n < frame_length; n++)
    {
        // first stage
        s1 = FB[0] * x[n] + FB[1] * state[0] + FB[2] * state[1] + FA[0] * state[2] + FA[1] * state[3];
        // second stage
        y = FB[0] * s1 + FB[1] * state[4] + FB[2] * state[5] + FA[0] * state[6] + FA[1] * state[7];

        //update state
        state[1] = state[0];
        state[0] = x[n];
        state[3] = state[2];
        state[2] = s1;

        state[5] = state[4];
        state[4] = s1;
        state[7] = state[6];
        state[6] = y;

        data_out[n] = y;
    }
}

void crossover_lpf_right(float *data_out, float *x, int frame_length, float *state, const float *FB, const float *FA)
{
    float s1;
    float y;
    int n;

    for (n = 0; n < frame_length; n++)
    {
        // first stage
        s1 = FB[0] * x[n] + FB[1] * state[0] + FB[2] * state[1] + FA[0] * state[2] + FA[1] * state[3];
        // second stage
        y = FB[0] * s1 + FB[1] * state[4] + FB[2] * state[5] + FA[0] * state[6] + FA[1] * state[7];

        //update state
        state[1] = state[0];
        state[0] = x[n];
        state[3] = state[2];
        state[2] = s1;

        state[5] = state[4];
        state[4] = s1;
        state[7] = state[6];
        state[6] = y;

        data_out[n] = y;
    }
}

void crossover_hpf_left(float *data_out, float *x, int frame_length, float *state, const float *FB, const float *FA)
{
    float s1;
    float y;
    int n;

    for (n = 0; n < frame_length; n++)
    {
        // first stage
        s1 = FB[0] * x[n] + FB[1] * state[0] + FB[2] * state[1] + FA[0] * state[2] + FA[1] * state[3];
        // second stage
        y = FB[0] * s1 + FB[1] * state[4] + FB[2] * state[5] + FA[0] * state[6] + FA[1] * state[7];

        //update state
        state[1] = state[0];
        state[0] = x[n];
        state[3] = state[2];
        state[2] = s1;

        state[5] = state[4];
        state[4] = s1;
        state[7] = state[6];
        state[6] = y;

        data_out[n] = y;
    }
}

void crossover_hpf_right(float *data_out, float *x, int frame_length, float *state, const float *FB, const float *FA)
{
    float s1;
    float y;
    int n;

    for (n = 0; n < frame_length; n++)
    {
        // first stage
        s1 = FB[0] * x[n] + FB[1] * state[0] + FB[2] * state[1] + FA[0] * state[2] + FA[1] * state[3];
        // second stage
        y = FB[0] * s1 + FB[1] * state[4] + FB[2] * state[5] + FA[0] * state[6] + FA[1] * state[7];

        //update state
        state[1] = state[0];
        state[0] = x[n];
        state[3] = state[2];
        state[2] = s1;

        state[5] = state[4];
        state[4] = s1;
        state[7] = state[6];
        state[6] = y;

        data_out[n] = y;
    }
}
