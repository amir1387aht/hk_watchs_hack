#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "arm_math.h"
#ifndef M_PI
    #define M_PI 3.141592653
#endif
//数学库
void performance_test(unsigned long iteration)
{
    unsigned long i = 0;
    float y1, y2;

    for (i = 1; i <= iteration; i++)
    {
        y1 = sinf(0.25);
        y2 = cosf(0.25);
    }
}

//hal接口
void hal_performance_test(unsigned long iteration)
{
    unsigned long i = 0;
    float y1, y2;
    int32_t cos, sin, angle;
    angle = (int32_t)(0.25 * M_PI * (1 << 31));
    for (i = 1; i <= iteration; i++)
    {
        HAL_CP_CosSin(angle, &cos, &sin);
        y1 = (float)((float)sin / (float)(1 << 31));
        y2 = (float)((float)cos / (float)(1 << 31));
    }
}

//CMSIS_DSP
void cmi_performance_test(unsigned long iteration)
{
    unsigned long i = 0;
    float y1, y2;


    for (i = 1; i <= iteration; i++)
    {
        y1 = arm_sin_f32(0.25 * M_PI);
        y2 = arm_cos_f32(0.25 * M_PI);
    }

}

int main(void)
{
    rt_kprintf("------------math math math math-----------\n");
    performance_test(10000000);
    rt_kprintf("-------------hal hal hal hal hal----------\n");
    hal_performance_test(10000000);
    rt_kprintf("----------------cmsis cmsis cmsis-------\n");
    cmi_performance_test(10000000);
    rt_kprintf("----------------ending ending ending-------\n");
    while (1)
    {
    }
    return 0;
}

