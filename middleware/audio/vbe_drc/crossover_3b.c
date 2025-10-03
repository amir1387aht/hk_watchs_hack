/*************************************************************
Crossover Filter Source Code
File: crossover_3b.c
*************************************************************/
#include <math.h>
#include "crossover_3b.h"

// one-stage biquid filter
// y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-1] + a1 * y[n-1] + a2* y[n-2]

void crossover_lpf1_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_hpf1_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_lpf2u_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_hpf2u_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_lpf2d_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_hpf2d_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_lpf1_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_hpf1_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_lpf2u_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_hpf2u_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_lpf2d_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}

void crossover_hpf2d_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA)
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
        s3 = FB[0] * s2 + FB[1] * state[8] + FB[2] * state[9] + FA[0] * state[10] + FA[1] * state[11];
        // fourth stage
        y = FB[3] * s3 + FB[4] * state[12] + FB[5] * state[13] + FA[2] * state[14] + FA[3] * state[15];

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

        data_out[n] = y;
    }
}
