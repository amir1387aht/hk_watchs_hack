/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stk8328c_reg.h"

int32_t stk8328c_read_reg(sifdev_sensor_ctx_t *ctx, uint8_t reg, uint8_t *data,
                          uint16_t len)
{
    int32_t ret;
    ret = ctx->read_reg(ctx->handle, reg, data, len);
    return ret;
}

int32_t stk8328c_write_reg(sifdev_sensor_ctx_t *ctx, uint8_t reg, uint8_t *data,
                           uint16_t len)
{
    int32_t ret;
    ret = ctx->write_reg(ctx->handle, reg, data, len);
    return ret;
}

int32_t stk8328c_device_id_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff)
{
    int32_t ret;
    ret = stk8328c_read_reg(ctx, STK_REG_CHIPID, buff, 1);
    return ret;
}

int32_t stk8328c_reset_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK_REG_SOFT_RESET, &val, 1);

    return ret;
}

int32_t stk8328c_xl_full_scale_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK832x_REG_RANGESEL, &val, 1);
    return ret;
}

int32_t stk8328c_fifo_mode_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK_REG_FIFO_MODE, &val, 1);
    return ret;
}

int32_t stk8328c_fifo_count_get(sifdev_sensor_ctx_t *ctx, uint8_t *buff)
{
    int32_t ret;
    ret = stk8328c_read_reg(ctx, STK_REG_FIFO_COUNT, buff, 1);
    return ret;
}

int32_t stk8328c_eng_mode_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK_REG_ENG_MODE, &val, 1);
    return ret;
}

int32_t stk8328c_i2c_wdt_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK_REG_I2C_WDT, &val, 1);
    return ret;
}

int32_t stk8328c_band_width_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK_REG_BAND_WIDTH, &val, 1);
    return ret;
}

int32_t stk8328c_es_mode_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK_REG_ES_MODE, &val, 1);
    return ret;
}

int32_t stk8328c_power_mode_set(sifdev_sensor_ctx_t *ctx, uint8_t val)
{
    int32_t ret;
    ret = stk8328c_write_reg(ctx, STK_REG_POWMODE, &val, 1);
    return ret;
}

int32_t stk8328c_read_fifo_data(sifdev_sensor_ctx_t *ctx, uint8_t *data, uint16_t len)
{
    int32_t ret;
    ret = stk8328c_read_reg(ctx, STK_REG_FIFO_DATA, data, len);
    return ret;
}

