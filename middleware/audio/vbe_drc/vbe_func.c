/*************************************************************
Virtual Bass Enhancement Source Code
File: vbe_func.c
*************************************************************/
#include <math.h>
#include "vbe_func.h"

void vbe_func(float vbe_gain, float *data_in, int frame_length)
{
    int n;

    float g_max, g_min, scale, offset, g;
    g_max = 4.0f;
    g_min = 1.0f;
    scale = (g_max - g_min) / (5.0f - 1.0f);
    offset = (-1) * (g_max - g_min) / (5.0f - 1.0f) + g_min;
    g = vbe_gain * scale + offset;

    for (n = 0; n < frame_length; n++)
    {
        if (data_in[n] > 0)
            data_in[n] *= g;
        else
            data_in[n] = 0;
    }

}
