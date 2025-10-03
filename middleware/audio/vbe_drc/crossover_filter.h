/********************************************************
File: CROSSOVER_FILTER.h
********************************************************/
#ifndef CROSSOVER_FILTER_H
#define CROSSOVER_FILTER_H


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void crossover_lpf_left(float *data_out, float *data_in, int frame_length, float *state, const float *FB, const float *FA);
void crossover_lpf_right(float *data_out, float *data_in, int frame_length, float *state, const float *FB, const float *FA);
void crossover_hpf_left(float *data_out, float *data_in, int frame_length, float *state, const float *FB, const float *FA);
void crossover_hpf_right(float *data_out, float *data_in, int frame_length, float *state, const float *FB, const float *FA);

#ifdef __cplusplus
}
#endif
#endif /* CROSSOVER_FILTER_H */
