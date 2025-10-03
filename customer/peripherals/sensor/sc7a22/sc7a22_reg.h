/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SC7A22_REG_H__
#define __SC7A22_REG_H__

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#define SC7A22_IC               0
#define SC7A20_IC               1

/*-----------------------------------------------------------------*/
#define SC7A22_I2C_ENABLE_REG    0x0E
#define SC7A22_WHO_AM_I_REG      0x0F
#define SC7A22_SOFT_RESET_REG    0x68
#define SC7A22_CTRL0_REG         0x1F
#define SC7A22_CTRL1_REG         0x20
#define SC7A22_CTRL2_REG         0x21
#define SC7A22_CTRL3_REG         0x22
#define SC7A22_CTRL4_REG         0x23
#define SC7A22_CTRL5_REG         0x24
#define SC7A22_CTRL6_REG         0x25
#define SC7A22_FIFO_CTRL_REG     0x2E
#define SC7A22_FIFO_STATUS_REG   0x2F
#define SC7A22_FIFO_DATA_REG     0x27 //0x69 is sc7a22, 0x27 is sc7a20

#define  SC7A22_I2C_ENABLE_VAL   0x04
#if SC7A20_IC
    #define  SC7A22_ID               0x11  //0x13 is sc7a22, 0x11 is sc7a20
#endif
#if SC7A22_IC
    #define  SC7A22_ID               0x13
#endif
#define  SC7A22_VERSION          0x22
#define  SC7A22_RESET_VAL        0xA5
#define  SC7A22_CTRL0_VAL        0x08
#define  SC7A22_CTRL1_VAL        0x47  // odr 50hz
#define  SC7A22_CTRL2_VAL        0x00
#define  SC7A22_CTRL3_VAL        0x00
#define  SC7A22_CTRL4_VAL        0x98  // scale 4g, spi 4 line
#define  SC7A22_CTRL5_VAL        0x40  // this val only for sc7a20
#define  SC7A22_CTRL6_VAL        0x00
#define  SC7A22_FIFO_MODE_VAL    0x4F  // fifo mode
#define  SC7A22_FIFO_BYPASS_VAL  0x00
#define  SC7A22_CLOSE_VAL        0x08  // 0x08 for sc7a20 

typedef int32_t (*stmdev_write_ptr)(void *, uint8_t, uint8_t *, uint16_t);
typedef int32_t (*stmdev_read_ptr)(void *, uint8_t, uint8_t *, uint16_t);

typedef struct
{
    /** Component mandatory fields **/
    stmdev_write_ptr  write_reg;
    stmdev_read_ptr   read_reg;
    /** Customizable optional pointer **/
    void *handle;
} sifdev_sensor_ctx_t;

int32_t sc7a22_read_reg(sifdev_sensor_ctx_t *ctx, uint8_t reg, uint8_t *data, uint16_t len);
int32_t sc7a22_write_reg(sifdev_sensor_ctx_t *ctx, uint8_t reg, uint8_t *data, uint16_t len);
int32_t sc7a22_device_id_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff);
int32_t sc7a22_reset_set(sifdev_sensor_ctx_t *ctx, uint8_t val);
int32_t sc7a22_xl_filter_set(sifdev_sensor_ctx_t *ctx, uint8_t val);
int32_t sc7a22_xl_data_rate_set(sifdev_sensor_ctx_t *ctx, uint8_t val);
int32_t sc7a22_i2c_enable_set(sifdev_sensor_ctx_t *ctx, uint8_t val);
int32_t sc7a22_xl_full_scale_set(sifdev_sensor_ctx_t *ctx, uint8_t val);
int32_t sc7a22_fifo_mode_set(sifdev_sensor_ctx_t *ctx, uint8_t val);
int32_t sc7a22_fifo_count_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff);
int32_t sc7a22_fifo_raw_data_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff, uint8_t len);
// note: ctrl reg5 val is diff with sc7a22 and sc7a20.
int32_t sc7a22_fifo_enable_set(sifdev_sensor_ctx_t *ctx, uint8_t val);



#ifdef __cplusplus
}
#endif
#endif /* __SC7A22_REG_H__*/

