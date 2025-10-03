#ifndef VBE_API_H
#define VBE_API_H



#ifdef __cplusplus
extern "C" {
#endif

#define VBE_ONE_FRAME_SAMPLES    32

void *vbe_drc_open(int samplerate, uint8_t channels, uint8_t bit_width);
int vbe_drc_process(void *vbe, int16_t *data, uint16_t samples, int16_t *out, uint32_t out_size);
void vbe_drc_close(void *vbe);

#ifdef __cplusplus
}
#endif
#endif
