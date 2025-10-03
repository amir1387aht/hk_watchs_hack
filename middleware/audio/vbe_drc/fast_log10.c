/*************************************************************
fast computation of log10
File: fast_log10.c
*************************************************************/
#include <stdint.h>
#include <math.h>
#include "fast_log10.h"

float fast_log10(float x)
{
    register union
    {
        float f;
        uint32_t i;
    } vx;
    register union
    {
        uint32_t i;
        float f;
    } mx;
    vx.f = x;
    mx.i = (vx.i & 0x007FFFFF) | 0x3F000000;
    return vx.i * 3.5885E-08f - 37.395f - 0.45095f * mx.f - 0.51954f / (0.35208f + mx.f);
}

