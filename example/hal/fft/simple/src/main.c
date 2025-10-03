/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bf0_hal.h"
#include "rtthread.h"
#include "stdio.h"
#include "string.h"

/* Test configuration */
#define FFT_LENGTH 64
#define SAMPLE_COUNT (FFT_LENGTH)
#define COMPLEX_SIZE (FFT_LENGTH * 2)

/* FFT handle */
static FFT_HandleTypeDef fft_handle;

/* Test data buffers - must be 4-byte aligned */
static ALIGN(4) uint16_t input_real[SAMPLE_COUNT] = {
    0xF11F, 0x0F86, 0xBA06, 0x22E4, 0x3ADD, 0x300D, 0x3558, 0x5063,
    0xCB6D, 0x7429, 0xCE99, 0x818A, 0x7E98, 0x026C, 0xE39B, 0x1675,
    0x2A86, 0x002C, 0x0AAD, 0x1A23, 0xF255, 0xFFA2, 0xC142, 0xD790,
    0x43B0, 0xB284, 0x1076, 0x9115, 0x2897, 0x0A3D, 0xE169, 0x91B9,
    0xF9CC, 0x4262, 0x6662, 0x498D, 0x71AE, 0x9284, 0x5A39, 0x295E,
    0xB8FE, 0x0017, 0x49BA, 0x60E6, 0xB799, 0x2242, 0x0B6D, 0x19E8,
    0xFC2F, 0xDDF3, 0x8149, 0x6AF7, 0x53B3, 0x2E6F, 0x61E4, 0x1A32,
    0xF212, 0x52B0, 0x7356, 0x3B6E, 0xF366, 0xE205, 0xAC22, 0xDB6A,
};
static ALIGN(4) uint16_t input_complex[COMPLEX_SIZE] = {
    0x0E72, 0x7466, 0x26DE, 0x6C74, 0xDE91, 0x13D3, 0x1C85, 0xA00E, 0x9770,
    0x8D74, 0x8999, 0x3280, 0x733A, 0x1CFF, 0x4667, 0xC3D2, 0xFA03, 0xC77E,
    0x62C7, 0x532B, 0xD887, 0x10E9, 0x4473, 0x50FD, 0xFED5, 0x12BA, 0xFB9D,
    0x1263, 0xF1E0, 0x6307, 0x0CDE, 0x6554, 0x134D, 0xFEEF, 0x1D06, 0x669A,
    0xA102, 0x753A, 0x9538, 0xCCB9, 0x4088, 0x077F, 0xA8E4, 0x82E8, 0xEE27,
    0xECD1, 0x8316, 0x75D0, 0x1CA4, 0xBA59, 0xEF53, 0x187C, 0xF5A5, 0x5BDC,
    0x2C74, 0x0EAC, 0x0582, 0x51E7, 0x7AA7, 0x0232, 0xBF09, 0x6564, 0x090D,
    0x37E1, 0xF6BD, 0x8192, 0xCB4C, 0x10A3, 0x7EB3, 0x7357, 0x3476, 0xE024,
    0x8CBB, 0xC221, 0x49B9, 0xC97A, 0x66F2, 0x414D, 0xB1F3, 0x1998, 0xA160,
    0x6B07, 0x06AE, 0xD997, 0x2378, 0xDE57, 0x122E, 0x9412, 0x4FFA, 0x4FE5,
    0x70AA, 0xE19A, 0xD755, 0xD971, 0x334D, 0xDEEB, 0x8A9B, 0x74CA, 0x2041,
    0x1057, 0x22A1, 0x0C67, 0xFF18, 0xDBB2, 0x0DC2, 0xFE7C, 0x7B94, 0x12D6,
    0x1E78, 0x4AAD, 0x866B, 0xA901, 0x1A19, 0x1903, 0xD2DD, 0x4876, 0x3682,
    0x91FC, 0x095D, 0xCD5E, 0xF9E6, 0x8531, 0x595F, 0x8C4D, 0xE6AD, 0x9A83,
    0x41C0, 0x2FE3,
};
static ALIGN(4) uint16_t output_buffer[COMPLEX_SIZE];
static ALIGN(4) uint16_t ifft_output[COMPLEX_SIZE];

static int fft_init(void)
{
    /* Enable FFT clock and select instance based on CPU */
#ifdef SOC_BF0_HCPU
    fft_handle.Instance = hwp_fft1;
    HAL_RCC_EnableModule(RCC_MOD_FFT1);
    rt_kprintf("FFT Example: Using FFT1 (HCPU)\n");
#else
    fft_handle.Instance = hwp_fft2;
    HAL_RCC_EnableModule(RCC_MOD_FFT2);
    rt_kprintf("FFT Example: Using FFT2 (LCPU)\n");
#endif

    /* Initialize FFT */
    HAL_StatusTypeDef status = HAL_FFT_Init(&fft_handle);
    if (status != HAL_OK)
    {
        rt_kprintf("FFT initialization failed: %d\n", status);
        return -1;
    }

    rt_kprintf("FFT initialization successful\n");
    return 0;
}

static void print_hex(const uint16_t *data, int count)
{
    for (int i = 0; i < count; i++)
    {
        rt_kprintf("[%d] = 0x%04X\n", i, data[i]);
    }
    rt_kprintf("\n");
}

static void test_real_fft(void)
{
    rt_kprintf("=== Real FFT Test ===\n");

    FFT_ConfigTypeDef config = {0};
    config.bitwidth = FFT_BW_16BIT;
    config.fft_length = FFT_LEN_64;
    config.rfft_flag = 1; /* Real FFT */
    config.ifft_flag = 0; /* Forward transform */
    config.input_data = input_real;
    config.output_data = output_buffer;

    /* Clear output buffer */
    memset(output_buffer, 0, sizeof(output_buffer));

    /* Execute FFT */
    HAL_StatusTypeDef status = HAL_FFT_StartFFT(&fft_handle, &config);
    if (status == HAL_OK)
    {
        rt_kprintf("Real FFT completed successfully\n");
        print_hex(output_buffer,
                  COMPLEX_SIZE); /* Print first 16 samples in hex */
    }
    else
    {
        rt_kprintf("Real FFT failed: %d\n", status);
    }
    rt_kprintf("\n");
}

static void test_complex_fft(void)
{
    rt_kprintf("=== Complex FFT Test ===\n");

    FFT_ConfigTypeDef config = {0};
    config.bitwidth = FFT_BW_16BIT;
    config.fft_length = FFT_LEN_64;
    config.rfft_flag = 0; /* Complex FFT */
    config.ifft_flag = 0; /* Forward transform */
    config.input_data = input_complex;
    config.output_data = output_buffer;

    /* Clear output buffer */
    memset(output_buffer, 0, sizeof(output_buffer));

    /* Execute FFT */
    HAL_StatusTypeDef status = HAL_FFT_StartFFT(&fft_handle, &config);
    if (status == HAL_OK)
    {
        rt_kprintf("Complex FFT completed successfully\n");
        print_hex(output_buffer, COMPLEX_SIZE);
    }
    else
    {
        rt_kprintf("Complex FFT failed: %d\n", status);
    }
    rt_kprintf("\n");
}

static void test_inverse_fft(void)
{
    rt_kprintf("=== Inverse FFT Test ===\n");

    /* First perform forward FFT to get frequency domain data */
    FFT_ConfigTypeDef config = {0};
    config.bitwidth = FFT_BW_16BIT;
    config.fft_length = FFT_LEN_64;
    config.rfft_flag = 0; /* Complex FFT */
    config.ifft_flag = 0; /* Forward transform */
    config.input_data = input_complex;
    config.output_data = output_buffer;

    memset(output_buffer, 0, sizeof(output_buffer));

    HAL_StatusTypeDef status = HAL_FFT_StartFFT(&fft_handle, &config);
    if (status != HAL_OK)
    {
        rt_kprintf("Forward FFT for IFFT test failed: %d\n", status);
        return;
    }

    config.ifft_flag = 1;              /* Inverse transform */
    config.input_data = output_buffer; /* Use FFT output as IFFT input */
    config.output_data = ifft_output;

    memset(ifft_output, 0, sizeof(ifft_output));

    status = HAL_FFT_StartFFT(&fft_handle, &config);
    if (status == HAL_OK)
    {
        rt_kprintf("Inverse FFT completed successfully\n");
        print_hex(output_buffer, COMPLEX_SIZE);
    }
    else
    {
        rt_kprintf("Inverse FFT failed: %d\n", status);
    }
    rt_kprintf("\n");
}

int main(void)
{
    rt_kprintf("\n");
    rt_kprintf("=========================================\n");
    rt_kprintf("        FFT HAL Module Example          \n");
    rt_kprintf("=========================================\n");
    rt_kprintf("This example demonstrates:\n");
    rt_kprintf("- Real FFT operation\n");
    rt_kprintf("- Complex FFT operation\n");
    rt_kprintf("- Inverse FFT operation\n");
    rt_kprintf("=========================================\n\n");

    /* Initialize FFT module */
    if (fft_init() != 0)
    {
        rt_kprintf("FFT module initialization failed\n");
        return -1;
    }

    /* Run tests */
    test_real_fft();
    test_complex_fft();
    test_inverse_fft();

    /* Cleanup */
    HAL_FFT_DeInit(&fft_handle);

    rt_kprintf("=========================================\n");
    rt_kprintf("          FFT Example Complete          \n");
    rt_kprintf("=========================================\n");

    return 0;
}
