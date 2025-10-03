/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"
#include "tc_utils.h"

#ifdef HAL_FFT_MODULE_ENABLED
#include "fft_512_hw_input.dat"
#include "fft_512_hw_output.dat"

/* Example Description:
 *
 * Use hwp_fft1 to calculate 512 point complex 16bit fixpoint FFT in polling and interrupt mode
 *
 */

static FFT_HandleTypeDef fft_handle;
volatile static uint8_t fft_done_flag;

void FFT1_IRQHandler(void)
{
    ENTER_INTERRUPT();
    HAL_FFT_IRQHandler(&fft_handle);
    LEAVE_INTERRUPT();
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void fft_done(FFT_HandleTypeDef *fft)
{
    fft_done_flag = 1;
}

static void init_fft(void)
{
    // Initialize driver and enable FFT IRQ
    HAL_NVIC_SetPriority(FFT1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(FFT1_IRQn);

    HAL_RCC_EnableModule(RCC_MOD_FFT1);

    fft_handle.Instance = hwp_fft1 ;
    HAL_FFT_Init(&fft_handle);
}

static void fft_polling_mode(void)
{
    FFT_ConfigTypeDef config;
    HAL_StatusTypeDef res;
    uint32_t data_size = 512 * 4;

    /* 初始化 */
    memset(&config, 0, sizeof(config));

    /* 1024点16比特的复数FFT */
    config.bitwidth = FFT_BW_16BIT;
    config.fft_length = FFT_LEN_512;
    config.ifft_flag = 0;
    config.rfft_flag = 0;
    config.input_data = rt_malloc(data_size);
    config.output_data = rt_malloc(data_size);
    RT_ASSERT(config.input_data);
    RT_ASSERT(config.output_data);
    memcpy(config.input_data, (void *)fft_512_hw_input, data_size);

    res = HAL_FFT_StartFFT(&fft_handle, &config);
    uassert_true_ret(res == HAL_OK);
    uassert_true_ret(0 == memcmp(config.output_data, (void *)fft_512_hw_output, data_size));

    rt_free(config.input_data);
    rt_free(config.output_data);
}

static void fft_interrupt_mode(void)
{
    FFT_ConfigTypeDef config;
    HAL_StatusTypeDef res;
    uint32_t data_size = 512 * 4;

    /* 初始化 */
    memset(&config, 0, sizeof(config));

    /* 1024点16比特的复数FFT */
    config.bitwidth = FFT_BW_16BIT;
    config.fft_length = FFT_LEN_512;
    config.ifft_flag = 0;
    config.rfft_flag = 0;
    config.input_data = rt_malloc(data_size);
    config.output_data = rt_malloc(data_size);
    RT_ASSERT(config.input_data);
    RT_ASSERT(config.output_data);
    memcpy(config.input_data, (void *)fft_512_hw_input, data_size);

    fft_done_flag = 0;
    fft_handle.CpltCallback = fft_done;

    res = HAL_FFT_StartFFT_IT(&fft_handle, &config);
    uassert_true_ret(res == HAL_OK);

    while (0 == fft_done_flag)
    {
    }

    uassert_true_ret(0 == memcmp(config.output_data, (void *)fft_512_hw_output, data_size));

    rt_free(config.input_data);
    rt_free(config.output_data);
}

static void testcase(int argc, char **argv)
{
    init_fft();
    UTEST_UNIT_RUN(fft_polling_mode);
    UTEST_UNIT_RUN(fft_interrupt_mode);

    return;
}

UTEST_TC_EXPORT(testcase, "example_fft", utest_tc_init, utest_tc_cleanup, 10);

#endif /* HAL_FFT_MODULE_ENABLED */

