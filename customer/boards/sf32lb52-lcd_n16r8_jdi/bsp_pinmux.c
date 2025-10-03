/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bsp_board.h"

#ifdef BSP_USING_PSRAM1
/* APS 128p*/
static void board_pinmux_psram_func0()
{
    HAL_PIN_Set(PAD_SA01, MPI1_DIO0, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA02, MPI1_DIO1, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA03, MPI1_DIO2, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA04, MPI1_DIO3, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA05, MPI1_DIO4, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA06, MPI1_DIO5, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA07, MPI1_DIO6, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA08, MPI1_DIO7, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA09, MPI1_DQSDM, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA10, MPI1_CLK,  PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_SA11, MPI1_CS,   PIN_NOPULL, 1);

    HAL_PIN_Set_Analog(PAD_SA00, 1);
    HAL_PIN_Set_Analog(PAD_SA12, 1);
}

/* APS 1:64p 2:32P, 4:Winbond 32/64/128p*/
static void board_pinmux_psram_func1_2_4(int func)
{
    HAL_PIN_Set(PAD_SA01, MPI1_DIO0, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA02, MPI1_DIO1, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA03, MPI1_DIO2, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA04, MPI1_DIO3, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA08, MPI1_DIO4, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA09, MPI1_DIO5, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA10, MPI1_DIO6, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA11, MPI1_DIO7, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA07, MPI1_CLK,  PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_SA05, MPI1_CS,   PIN_NOPULL, 1);

#ifdef FPGA
    HAL_PIN_Set(PAD_SA00, MPI1_DM, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA06, MPI1_CLKB, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_SA12, MPI1_DQSDM, PIN_PULLDOWN, 1);
#else
    switch (func)
    {
    case 1:             // APS 64P XCELLA
        HAL_PIN_Set(PAD_SA12, MPI1_DQSDM, PIN_PULLDOWN, 1);
        HAL_PIN_Set_Analog(PAD_SA00, 1);
        HAL_PIN_Set_Analog(PAD_SA06, 1);
        break;
    case 2:             // APS 32P LEGACY
        HAL_PIN_Set(PAD_SA00, MPI1_DM, PIN_PULLDOWN, 1);
        HAL_PIN_Set(PAD_SA12, MPI1_DQS, PIN_PULLDOWN, 1);
        HAL_PIN_Set(PAD_SA06, MPI1_CLKB, PIN_NOPULL, 1);
        break;
    case 4:             // Winbond 32/64/128p
        //HAL_PIN_Set(PAD_SA06, MPI1_CLKB, PIN_NOPULL, 1);
        HAL_PIN_Set(PAD_SA12, MPI1_DQSDM, PIN_NOPULL, 1);
        HAL_PIN_Set_Analog(PAD_SA00, 1);
        HAL_PIN_Set_Analog(PAD_SA06, 1);
        break;
    }
#endif
}

/* APS 16p*/
static void board_pinmux_psram_func3()
{
    HAL_PIN_Set(PAD_SA09, MPI1_CLK, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_SA08, MPI1_CS,  PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_SA05, MPI1_DIO0, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA07, MPI1_DIO1, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_SA06, MPI1_DIO2, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_SA10, MPI1_DIO3, PIN_PULLUP, 1);

    HAL_PIN_Set_Analog(PAD_SA00, 1);
    HAL_PIN_Set_Analog(PAD_SA01, 1);
    HAL_PIN_Set_Analog(PAD_SA02, 1);
    HAL_PIN_Set_Analog(PAD_SA03, 1);
    HAL_PIN_Set_Analog(PAD_SA04, 1);
    HAL_PIN_Set_Analog(PAD_SA11, 1);
    HAL_PIN_Set_Analog(PAD_SA12, 1);
}

static void board_pinmux_mpi1_none(void)
{
    uint32_t i;

    for (i = 0; i <= 12; i++)
    {
        HAL_PIN_Set_Analog(PAD_SA00 + i, 1);
    }
}
#endif

static void BSP_PIN_Common(void)
{
#ifdef SOC_BF0_HCPU
    // HCPU pins

    uint32_t pid = (hwp_hpsys_cfg->IDR & HPSYS_CFG_IDR_PID_Msk) >> HPSYS_CFG_IDR_PID_Pos;

    pid &= 7;

#ifdef BSP_USING_PSRAM1
    switch (pid)
    {
    case 5: //BOOT_PSRAM_APS_16P:
        board_pinmux_psram_func3();         // 16Mb APM QSPI PSRAM
        break;
    case 4: //BOOT_PSRAM_APS_32P:
        board_pinmux_psram_func1_2_4(2);    // 32Mb APM LEGACY PSRAM
        break;
    case 6: //BOOT_PSRAM_WINBOND:                // Winbond HYPERBUS PSRAM
        board_pinmux_psram_func1_2_4(4);
        break;
    case 3: // BOOT_PSRAM_APS_64P:
        board_pinmux_psram_func1_2_4(1);    // 64Mb APM XCELLA PSRAM
        break;
    case 2: //BOOT_PSRAM_APS_128P:
        board_pinmux_psram_func0();         // 128Mb APM XCELLA PSRAM
        break;
    default:
        board_pinmux_mpi1_none();
        break;
    }
#endif /* BSP_USING_PSRAM1 */

#ifdef BSP_ENABLE_MPI2
    // MPI2
    HAL_PIN_Set(PAD_PA16, MPI2_CLK,  PIN_NOPULL,   1);
    HAL_PIN_Set(PAD_PA12, MPI2_CS,   PIN_NOPULL,   1);
    HAL_PIN_Set(PAD_PA15, MPI2_DIO0, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_PA13, MPI2_DIO1, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_PA14, MPI2_DIO2, PIN_PULLUP,   1);
    HAL_PIN_Set(PAD_PA17, MPI2_DIO3, PIN_PULLUP, 1);
#elif BSP_USING_SDIO
    HAL_PIN_Set(PAD_PA15, SD1_CMD, PIN_PULLUP, 1);
    HAL_Delay_us(20);
    HAL_PIN_Set(PAD_PA12, SD1_DIO2,  PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA13, SD1_DIO3, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA14, SD1_CLK, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA16, SD1_DIO0, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA17, SD1_DIO1, PIN_PULLUP, 1);
#endif
    HAL_PIN_Set(PAD_PA00, GPIO_A0,  PIN_PULLDOWN, 1);     // #LCD_RESETB
    HAL_PIN_Set(PAD_PA10, GPIO_A10, PIN_PULLDOWN, 1);     // AUDIO_PA_CTRL

    // UART1 - debug
    HAL_PIN_Set(PAD_PA18, USART1_RXD, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA19, USART1_TXD, PIN_PULLUP, 1);

    // UART2 - log
    HAL_PIN_Set(PAD_PA20, USART2_RXD, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA27, USART2_TXD, PIN_PULLUP, 1);

    // Key1 - Power key
    /* Keep default pull-down unchanged. Uart download driver would use this function,
     * if pulldown is disabled, download driver would not work on the board without external pull-down
     */
    // HAL_PIN_Set(PAD_PA34, GPIO_A34, PIN_NOPULL, 1);
    // Key2
    HAL_PIN_Set(PAD_PA11, GPIO_A11, PIN_NOPULL, 1);

    // PA22 #XTAL32K_XI
    // PA23 #XTAL32K_XO

    // USBD
    HAL_PIN_Set_Analog(PAD_PA35, 1);                    // USB_DP
    HAL_PIN_Set_Analog(PAD_PA36, 1);                    // USB_DM

    // SPI1(TF card)
    HAL_PIN_Set(PAD_PA24, SPI1_DIO, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA25, SPI1_DI,  PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_PA28, SPI1_CLK, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA29, SPI1_CS,  PIN_NOPULL, 1);

//     HAL_PIN_Set_DS0(PAD_PA24, 1, 1);
//     HAL_PIN_Set_DS0(PAD_PA25, 1, 1);
//     HAL_PIN_Set_DS0(PAD_PA28, 1, 1);
//     HAL_PIN_Set_DS0(PAD_PA29, 1, 1);
//
//     HAL_PIN_Set_DS1(PAD_PA24, 1, 1);
//     HAL_PIN_Set_DS1(PAD_PA25, 1, 1);
//     HAL_PIN_Set_DS1(PAD_PA28, 1, 1);
//     HAL_PIN_Set_DS1(PAD_PA29, 1, 1);
#if defined(BSP_USING_PWM3) || defined(BSP_USING_RGBLED_WITCH_PWM3)
    HAL_PIN_Set(PAD_PA32, GPTIM2_CH1, PIN_PULLUP, 1);   // RGB LED
#endif
    // GPIOs
    HAL_PIN_Set(PAD_PA21, GPIO_A21, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_PA26, GPIO_A26, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA32, GPIO_A32, PIN_PULLDOWN, 1);   // RGB LED
    HAL_PIN_Set(PAD_PA38, GPIO_A38, PIN_PULLDOWN, 1);
    HAL_PIN_Set(PAD_PA44, GPIO_A44, PIN_PULLDOWN, 1);   // VBUS_DET
#endif

}

void BSP_PIN_Touch(void)
{
    // Touch
    HAL_PIN_Set(PAD_PA09, GPIO_A9,  PIN_NOPULL, 1);    // CTP_RESET
    HAL_PIN_Set(PAD_PA31, GPIO_A31, PIN_NOPULL, 1);    // CTP_INT
    HAL_PIN_Set(PAD_PA30, I2C1_SCL, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA33, I2C1_SDA, PIN_PULLUP, 1);
}

void BSP_PIN_LCD(void)
{

#ifdef BSP_LCDC_USING_JDI_PARALLEL
    HAL_PIN_Set(PAD_PA02, LCDC1_JDI_B2, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA03, LCDC1_JDI_B1, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA04, LCDC1_JDI_G1, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA05, LCDC1_JDI_R1, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA06, LCDC1_JDI_HST, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA07, LCDC1_JDI_ENB, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA08, LCDC1_JDI_VST, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA39, LCDC1_JDI_VCK, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA40, LCDC1_JDI_XRST, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA41, LCDC1_JDI_HCK, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA42, LCDC1_JDI_R2, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA43, LCDC1_JDI_G2, PIN_NOPULL, 1);

    HAL_PIN_Set(PAD_PA24, PBR_LPTIM2_OUT, PIN_NOPULL, 1);  //FRP
    HAL_PIN_Set(PAD_PA25, PBR_LPTIM2_INV_OUT, PIN_NOPULL, 1);  //XFRP

#else
    /* disable compile error as LCD may be disabled by some example, such as hal_example */
// #error LCD type not supported in this board.
#endif

}

void BSP_PIN_Init(void)
{
    BSP_PIN_Common();

    BSP_PIN_LCD();

}

