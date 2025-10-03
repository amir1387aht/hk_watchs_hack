/********************************************************
File: CROSSOVER_3B.h
********************************************************/
#ifndef CROSSOVER_3B_H
#define CROSSOVER_3B_H



#ifdef __cplusplus
extern "C" {
#endif

void crossover_lpf1_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_hpf1_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_lpf2u_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_hpf2u_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_lpf2d_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_hpf2d_left(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);

void crossover_lpf1_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_hpf1_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_lpf2u_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_hpf2u_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_lpf2d_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);
void crossover_hpf2d_right(float *data_out, float *x, int frame_length, float *state, float *FB, float *FA);

#ifdef __cplusplus
}
#endif
#endif /* CROSSOVER_3B_H */
