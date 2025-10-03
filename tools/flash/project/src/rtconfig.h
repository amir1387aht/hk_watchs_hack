#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* Sifli Configuration */

#define SOC_SF32LB55X
#define CORE "HCPU"
#define CPU "Cortex-M33"

/* RTOS */

#define BSP_USING_NO_OS
#define RT_USING_LIBC
#ifdef SF32LB55X
    #define BSP_USING_QSPI
    #define BSP_USING_SPI_FLASH
    #define BSP_ENABLE_QSPI1
    #define BSP_QSPI1_USING_DMA
    #define BSP_QSPI1_MODE 0
    #define BSP_QSPI1_MEM_SIZE 8
    #define BSP_ENABLE_QSPI2
    #define BSP_QSPI2_USING_DMA
    #define BSP_QSPI2_MODE 0
    #define BSP_QSPI2_MEM_SIZE 32
    #define BSP_ENABLE_QSPI3
    #define BSP_QSPI3_USING_DMA
    #define BSP_QSPI3_MODE 1
    #define BSP_QSPI3_MEM_SIZE 128
    #define BSP_ENABLE_QSPI4
    #define BSP_QSPI4_USING_DMA
    #define BSP_QSPI4_MODE 0
    #define BSP_QSPI4_MEM_SIZE 4
#elif SF32LB58X
    #define BSP_USING_QSPI
    #define BSP_USING_SPI_FLASH
    #define BSP_ENABLE_QSPI1
    #define BSP_QSPI1_USING_DMA
    #define BSP_QSPI1_MODE 0
    #define BSP_QSPI1_MEM_SIZE 8
    #define BSP_ENABLE_QSPI2
    #define BSP_QSPI2_USING_DMA
    #define BSP_QSPI2_MODE 0
    #define BSP_QSPI2_MEM_SIZE 32
#else
    #define BSP_USING_FLASH
    #define BSP_ENABLE_FLASH1
    #define BSP_FLASH1_USING_DMA
    #define BSP_FLASH1_MEM_SIZE 2
    #define BSP_ENABLE_FLASH2
    #define BSP_FLASH2_USING_DMA
    #define BSP_FLASH2_MEM_SIZE 32
#endif
/* Select board peripherals */

#define BSP_USING_BOARD_FPGA_A0

/* Key config */

/* Sifli Middleware */

#define BSP_USING_EMPTY_ASSERT

/* RW Bluetooth */


/* Third party packages */

#define BF0_HCPU


#define rt_hw_interrupt_enable(level)
#define rt_hw_interrupt_disable()  0
#define RT_EOK      0
#define RT_ERROR 1
#define JLINK_lOG 1


#endif
