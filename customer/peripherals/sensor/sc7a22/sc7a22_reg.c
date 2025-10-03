/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sc7a22_reg.h"

int32_t sc7a22_read_reg(sifdev_sensor_ctx_t *ctx, uint8_t reg, uint8_t *data,
                        uint16_t len)
{
    int32_t ret;
    ret = ctx->read_reg(ctx->handle, reg, data, len);
    return ret;
}

int32_t sc7a22_write_reg(sifdev_sensor_ctx_t *ctx, uint8_t reg, uint8_t *data,
                         uint16_t len)
{
    int32_t ret;
    ret = ctx->write_reg(ctx->handle, reg, data, len);
    return ret;
}

int32_t sc7a22_device_id_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff)
{
    int32_t ret;
    ret = sc7a22_read_reg(ctx, SC7A22_WHO_AM_I_REG, buff, 1);
    return ret;
}

int32_t sc7a22_reset_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = sc7a22_write_reg(ctx, SC7A22_SOFT_RESET_REG, &val, 1);

    return ret;
}

int32_t sc7a22_xl_filter_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = sc7a22_write_reg(ctx, SC7A22_CTRL0_REG, &val, 1);

    return ret;
}

int32_t sc7a22_xl_data_rate_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = sc7a22_write_reg(ctx, SC7A22_CTRL1_REG, &val, 1);

    return ret;

}

int32_t sc7a22_i2c_enable_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = sc7a22_write_reg(ctx, SC7A22_I2C_ENABLE_REG, &val, 1);

    return ret;
}

int32_t sc7a22_xl_full_scale_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = sc7a22_write_reg(ctx, SC7A22_CTRL4_REG, &val, 1);
    return ret;
}

int32_t sc7a22_fifo_mode_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = sc7a22_write_reg(ctx, SC7A22_FIFO_CTRL_REG, &val, 1);
    return ret;
}

int32_t sc7a22_fifo_count_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff)
{
    int32_t ret;
    ret = sc7a22_read_reg(ctx, SC7A22_FIFO_STATUS_REG, buff, 1);
    return ret;
}

int32_t sc7a22_fifo_raw_data_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff, uint8_t len)
{
    int32_t ret;
    ret = sc7a22_read_reg(ctx, SC7A22_FIFO_DATA_REG, buff, len);
    return ret;
}

// note: ctrl reg5 val is diff with sc7a22 and sc7a20.
int32_t sc7a22_fifo_enable_set(sifdev_sensor_ctx_t *ctx, uint8_t val)

{
    int32_t ret;
    ret = sc7a22_write_reg(ctx, SC7A22_CTRL5_REG, &val, 1);
    return ret;
}

