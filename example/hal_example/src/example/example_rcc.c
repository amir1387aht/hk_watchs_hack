/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>
#include <string.h>
#include "utest.h"
#include "bf0_hal.h"

#ifdef HAL_RCC_MODULE_ENABLED
/*
    This example demo:
    1. Reset HCPU on-chip peripherals
    2. Switch system to use 48M external Crystal.
    3. Enable DLL1 to use 192M Hz
    4. Switch system clock to DLL1
*/

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void aes_async_cbk(void)
{
    LOG_I("Aes complete\n");
}

#ifdef SOC_SF32LB55X
#define HCPU_RESET_MODULES  \
    (HPSYS_RCC_RSTR1_MAILBOX1|\
     HPSYS_RCC_RSTR1_USART2|\
     HPSYS_RCC_RSTR1_EZIP|\
     HPSYS_RCC_RSTR1_EPIC|\
     HPSYS_RCC_RSTR1_LCDC1|\
     HPSYS_RCC_RSTR1_I2S1|\
     HPSYS_RCC_RSTR1_I2S2|\
     HPSYS_RCC_RSTR1_CRC |\
     HPSYS_RCC_RSTR1_TRNG|\
     HPSYS_RCC_RSTR1_GPTIM1|\
     HPSYS_RCC_RSTR1_GPTIM2|\
     HPSYS_RCC_RSTR1_BTIM1|\
     HPSYS_RCC_RSTR1_BTIM2|\
     HPSYS_RCC_RSTR1_SPI1|\
     HPSYS_RCC_RSTR1_SPI2|\
     HPSYS_RCC_RSTR1_PSRAMC|\
     HPSYS_RCC_RSTR1_NNACC|\
     HPSYS_RCC_RSTR1_PDM1|\
     HPSYS_RCC_RSTR1_PDM2|\
     HPSYS_RCC_RSTR1_I2C1|\
     HPSYS_RCC_RSTR1_I2C2|\
     HPSYS_RCC_RSTR1_DSIHOST|\
     HPSYS_RCC_RSTR1_DSIPHY|\
     HPSYS_RCC_RSTR1_PTC1\
    )

#define HCPU_RESET_MODULES2  \
    (HPSYS_RCC_RSTR2_I2C3|\
     HPSYS_RCC_RSTR2_BUSMON1|\
     HPSYS_RCC_RSTR2_USBC|\
     HPSYS_RCC_RSTR2_SDMMC2|\
     HPSYS_RCC_RSTR2_SDMMC1|\
     HPSYS_RCC_RSTR2_QSPI3\
     )
#elif defined(SOC_SF32LB56X)
#define HCPU_RESET_MODULES  \
    (HPSYS_RCC_RSTR1_MAILBOX1|\
     HPSYS_RCC_RSTR1_USART2|\
     HPSYS_RCC_RSTR1_EZIP1|\
     HPSYS_RCC_RSTR1_EPIC|\
     HPSYS_RCC_RSTR1_LCDC1|\
     HPSYS_RCC_RSTR1_I2S1|\
     HPSYS_RCC_RSTR1_CRC1 |\
     HPSYS_RCC_RSTR1_TRNG|\
     HPSYS_RCC_RSTR1_GPTIM1|\
     HPSYS_RCC_RSTR1_GPTIM2|\
     HPSYS_RCC_RSTR1_BTIM1|\
     HPSYS_RCC_RSTR1_BTIM2|\
     HPSYS_RCC_RSTR1_SPI1|\
     HPSYS_RCC_RSTR1_SPI2|\
     HPSYS_RCC_RSTR1_NNACC1|\
     HPSYS_RCC_RSTR1_PDM1|\
     HPSYS_RCC_RSTR1_PDM2|\
     HPSYS_RCC_RSTR1_I2C1|\
     HPSYS_RCC_RSTR1_I2C2|\
     HPSYS_RCC_RSTR1_PTC1\
    )

#define HCPU_RESET_MODULES2  \
    (HPSYS_RCC_RSTR2_I2C3|\
     HPSYS_RCC_RSTR2_BUSMON1|\
     HPSYS_RCC_RSTR2_USBC|\
     HPSYS_RCC_RSTR2_SDMMC2|\
     HPSYS_RCC_RSTR2_SDMMC1|\
     HPSYS_RCC_RSTR2_BUSMON1|\
     HPSYS_RCC_RSTR2_ATIM1|\
     HPSYS_RCC_RSTR2_FFT1|\
     HPSYS_RCC_RSTR2_FACC1|\
     HPSYS_RCC_RSTR2_SCI|\
     HPSYS_RCC_RSTR2_CAN1|\
     HPSYS_RCC_RSTR2_AUDCODEC|\
     HPSYS_RCC_RSTR2_AUDPRC|\
     HPSYS_RCC_RSTR2_I2C4\
     )

#elif defined(SOC_SF32LB58X)
#define HCPU_RESET_MODULES  \
    (HPSYS_RCC_RSTR1_MAILBOX1|\
     HPSYS_RCC_RSTR1_USART2|\
     HPSYS_RCC_RSTR1_EZIP1|\
     HPSYS_RCC_RSTR1_EPIC|\
     HPSYS_RCC_RSTR1_LCDC1|\
     HPSYS_RCC_RSTR1_I2S1|\
     HPSYS_RCC_RSTR1_I2S2|\
     HPSYS_RCC_RSTR1_CRC1 |\
     HPSYS_RCC_RSTR1_TRNG|\
     HPSYS_RCC_RSTR1_GPTIM1|\
     HPSYS_RCC_RSTR1_GPTIM2|\
     HPSYS_RCC_RSTR1_BTIM1|\
     HPSYS_RCC_RSTR1_BTIM2|\
     HPSYS_RCC_RSTR1_SPI1|\
     HPSYS_RCC_RSTR1_SPI2|\
     HPSYS_RCC_RSTR1_NNACC1|\
     HPSYS_RCC_RSTR1_PDM1|\
     HPSYS_RCC_RSTR1_PDM2|\
     HPSYS_RCC_RSTR1_I2C1|\
     HPSYS_RCC_RSTR1_I2C2|\
     HPSYS_RCC_RSTR1_DSIHOST|\
     HPSYS_RCC_RSTR1_DSIPHY|\
     HPSYS_RCC_RSTR1_PTC1\
    )

#define HCPU_RESET_MODULES2  \
    (HPSYS_RCC_RSTR2_I2C3|\
     HPSYS_RCC_RSTR2_BUSMON1|\
     HPSYS_RCC_RSTR2_USBC|\
     HPSYS_RCC_RSTR2_SDMMC2|\
     HPSYS_RCC_RSTR2_SDMMC1|\
     HPSYS_RCC_RSTR2_JENC|\
     HPSYS_RCC_RSTR2_JDEC|\
     HPSYS_RCC_RSTR2_I2C4|\
     HPSYS_RCC_RSTR2_ACPU|\
     )
#elif defined(SOC_SF32LB52X)
#define HCPU_RESET_MODULES  \
    (HPSYS_RCC_RSTR1_MAILBOX1|\
     HPSYS_RCC_RSTR1_USART2|\
     HPSYS_RCC_RSTR1_EZIP1|\
     HPSYS_RCC_RSTR1_EPIC|\
     HPSYS_RCC_RSTR1_LCDC1|\
     HPSYS_RCC_RSTR1_I2S1|\
     HPSYS_RCC_RSTR1_CRC1 |\
     HPSYS_RCC_RSTR1_TRNG|\
     HPSYS_RCC_RSTR1_GPTIM1|\
     HPSYS_RCC_RSTR1_GPTIM2|\
     HPSYS_RCC_RSTR1_BTIM1|\
     HPSYS_RCC_RSTR1_BTIM2|\
     HPSYS_RCC_RSTR1_SPI1|\
     HPSYS_RCC_RSTR1_SPI2|\
     HPSYS_RCC_RSTR1_PDM1|\
     HPSYS_RCC_RSTR1_I2C1|\
     HPSYS_RCC_RSTR1_I2C2|\
     HPSYS_RCC_RSTR1_PTC1\
    )

#define HCPU_RESET_MODULES2  \
    (HPSYS_RCC_RSTR2_I2C3|\
     HPSYS_RCC_RSTR2_USBC|\
     HPSYS_RCC_RSTR2_SDMMC1|\
     HPSYS_RCC_RSTR2_I2C4\
     )
#endif
static void testcase(int argc, char **argv)
{
    HAL_StatusTypeDef ret;

    LOG_I("Reset peripherals on HCPU");
    HAL_RCC_HCPU_reset(HCPU_RESET_MODULES, 1);                          // Reset HCPU on-chip peripherals
#ifdef SF32LB55X
    HAL_RCC_HCPU_reset2(HCPU_RESET_MODULES2, 1);                          // Reset HCPU on-chip peripherals
#endif
    HAL_HPAON_EnableXT48();
    HAL_RCC_HCPU_reset(HCPU_RESET_MODULES, 0);
#ifdef SF32LB55X
    HAL_RCC_HCPU_reset2(HCPU_RESET_MODULES2, 0);                          // Reset HCPU on-chip peripherals
#endif

    HAL_RCC_HCPU_ClockSelect(RCC_CLK_MOD_SYS, RCC_SYSCLK_HXT48);      // Switch system to use 48M external Crystal.
    ret = HAL_RCC_HCPU_EnableDLL1(192000000);                         // Enable DLL1 to use 192M Hz
    RT_ASSERT(HAL_OK == ret);
    LOG_I("Set system clock to use clock of DLL1 192M");
    HAL_RCC_HCPU_ClockSelect(RCC_CLK_MOD_SYS, RCC_SYSCLK_DLL1);         // Switch system clock to DLL1
    LOG_I("Now system clock is 192M");
}

UTEST_TC_EXPORT(testcase, "example_rcc", utest_tc_init, utest_tc_cleanup, 10);
#endif

