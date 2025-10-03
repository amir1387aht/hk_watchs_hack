/*************************************************************
Audio EQ Filter Source Code
File: eq_filter.c
*************************************************************/
#include <math.h>
#include "eq_filter.h"

// one-stage biquid filter
// y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-1] + a1 * y[n-1] + a2* y[n-2]

void eq_coef2float(float *eq_coef_f, int32_t *eq_coef, int eq_num_stage)
{
    int n;
    int coef_len;

    coef_len = 5 * eq_num_stage;

    for (n = 0; n < coef_len; n++)
    {
        if (eq_coef[n] < (1 << 23))
            eq_coef_f[n] = (float)eq_coef[n];
        else
            eq_coef_f[n] = (float)eq_coef[n] - ((1 << 24));

        eq_coef_f[n] /= (1 << 23);
    }
}

void eq_filter_left(float *data_out, float *data_in, int frame_length, float *state, float *coef, int eq_num_stage)
{
    float s[32];
    float x, m0, m1, m2, s1, s2;
    int stage_index, n;


    for (n = 0; n < frame_length; n++)
    {
        for (stage_index = 0; stage_index < eq_num_stage; stage_index++)
        {

            if (stage_index == 0)
            {
                x = data_in[n];
            }
            else
            {
                x = s[stage_index - 1];
            }

            m0 = x * coef[stage_index * 5 + 0];

            s[stage_index] = (m0 + state[stage_index * 2 + 0]) * 4.0f;

            m1 = x * coef[stage_index * 5 + 1];
            m2 = x * coef[stage_index * 5 + 3];

            s1 = s[stage_index] * coef[stage_index * 5 + 2];
            s2 = s[stage_index] * coef[stage_index * 5 + 4];

            // update state
            state[stage_index * 2 + 0] = m1 + state[stage_index * 2 + 1] + s1;
            state[stage_index * 2 + 1] = m2 + s2;
        }

        // output
        data_out[n] = s[eq_num_stage - 1];
    }
}

void eq_filter_right(float *data_out, float *data_in, int frame_length, float *state, float *coef, int eq_num_stage)
{
    float s[32];
    float x, m0, m1, m2, s1, s2;
    int stage_index, n;


    for (n = 0; n < frame_length; n++)
    {
        for (stage_index = 0; stage_index < eq_num_stage; stage_index++)
        {

            if (stage_index == 0)
            {
                x = data_in[n];
            }
            else
            {
                x = s[stage_index - 1];
            }

            m0 = x * coef[stage_index * 5 + 0];

            s[stage_index] = (m0 + state[stage_index * 2 + 0]) * 4.0f;

            m1 = x * coef[stage_index * 5 + 1];
            m2 = x * coef[stage_index * 5 + 3];

            s1 = s[stage_index] * coef[stage_index * 5 + 2];
            s2 = s[stage_index] * coef[stage_index * 5 + 4];

            // update state
            state[stage_index * 2 + 0] = m1 + state[stage_index * 2 + 1] + s1;
            state[stage_index * 2 + 1] = m2 + s2;
        }

        // output
        data_out[n] = s[eq_num_stage - 1];
    }
}