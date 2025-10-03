/********************************************************
File: VBE_FUNC.h
********************************************************/
#ifndef VBE_FUNC_H
#define VBE_FUNC_H


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void vbe_func(float   vbe_gain, float *data_in, int frame_length);

#ifdef __cplusplus
}
#endif
#endif /* VBE_FUNC_H */
