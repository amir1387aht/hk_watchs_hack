#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* Sifli Configuration */

#define SOC_SF32LB56X 1

#define CORE "HCPU"
#define CPU "Cortex-M33"

/* RTOS */

#define BSP_USING_NO_OS
#define RT_USING_LIBC
#define BSP_USING_MPI 1
#define BSP_USING_QSPI 1
#define BSP_USING_SPI_FLASH 1
#define BSP_ENABLE_MPI3 1
#define BSP_ENABLE_QSPI3 1
#define BSP_MPI3_MODE_1 1
#define BSP_QSPI3_MODE 1
#define BSP_QSPI3_USING_DMA 1
#define BSP_USING_NAND_FLASH3 1
#define BSP_QSPI3_MEM_SIZE 128
#define BSP_QSPI3_CHIP_ID 0
#define BSP_ENABLE_QSPI5 1
#define BSP_QSPI5_USING_DMA 1
#define BSP_QSPI5_MODE_0 1
#define BSP_QSPI5_MODE 0
#define BSP_USING_NOR_FLASH5 1
#define BSP_QSPI5_MEM_SIZE 1
#define BSP_QSPI5_CHIP_ID 0

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

#endif
