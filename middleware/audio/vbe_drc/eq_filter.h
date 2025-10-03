/********************************************************
File: eq_filter.h
********************************************************/
#ifndef EQ_FILTER_H
#define EQ_FILTER_H


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void eq_coef2float(float *eq_coef_f, int32_t *eq_coef, int eq_num_stage);
void eq_filter_left(float *data_out, float *data_in, int frame_length, float *state, float *coef, int eq_num_stage);
void eq_filter_right(float *data_out, float *data_in, int frame_length, float *state, float *coef, int eq_num_stage);

#ifdef __cplusplus
}
#endif
#endif /* EQ_FILTER_H */
