// mem_map.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "stdio.h"
#include "mem_map.h"


unsigned int addresses[] =
{
    // HCPU system in FPGA ---------------------------------------------------------
    BOOTLOADER_CODE_START_ADDR, BOOTLOADER_CODE_SIZE,           // Boot ROM
    HCPU_CODE_START_ADDR, HCPU_CODE_SIZE,                       // RO_CODE
    HCPU_RO_DATA_START_ADDR, HCPU_RO_DATA_SIZE,                 // RO_DATA
    HCPU_RAM_DATA_START_ADDR, HCPU_RAM_DATA_SIZE,               // RAM
    HCPU2BCPU_MB_CH2_BUF_START_ADDR, HCPU2BCPU_MB_CH2_BUF_SIZE, // HCPU->BCPU, Channel 2
    HCPU2LCPU_MB_CH2_BUF_START_ADDR, HCPU2LCPU_MB_CH2_BUF_SIZE, // HCPU->LCPU, Channel 2
    HCPU2BCPU_MB_CH1_BUF_START_ADDR, HCPU2BCPU_MB_CH1_BUF_SIZE, // HCPU->BCPU, Channel 1
    HCPU2LCPU_MB_CH1_BUF_START_ADDR, HCPU2LCPU_MB_CH1_BUF_SIZE, // HCPU->LCPU, Channel 1
    HPSYS_RAM0_BASE, HPSYS_RAM_SIZE,                                         // HCPU Total RAM


    // LCPU in FPGA,---------------------------------------------------------------
    LCPU_CODE_START_ADDR, LCPU_CODE_SIZE,                       // LCPU RO
    LCPU_RAM_DATA_START_ADDR, LCPU_RAM_DATA_SIZE,               // LCPU RAM
    LCPU2HCPU_MB_CH1_BUF_START_ADDR, LCPU2HCPU_MB_CH1_BUF_SIZE, // LCPU->HCPU, Channel 1
    LCPU2HCPU_MB_CH2_BUF_START_ADDR, LCPU2HCPU_MB_CH2_BUF_SIZE, // LCPU->HCPU, Channel 2
    LCPU_ROM_CODE_START_ADDR, LCPU_ROM_CODE_SIZE,               // LCPU Boot ROM
    LPSYS_RAM_BASE, LPSYS_RAM_SIZE,                             // LCPU Total RAM

    // BCPU in FPGA, --------------------------------------------------------------------
    BCPU_CODE_START_ADDR, BCPU_CODE_SIZE,                       // RO
    BCPU_RAM_DATA_START_ADDR, BCPU_RAM_DATA_SIZE,               // RAM
    // Mailbox buffer
    BCPU2HCPU_MB_CH1_BUF_START_ADDR, BCPU2HCPU_MB_CH1_BUF_SIZE, // BCPU->HCPU, Channel 1
    BCPU2HCPU_MB_CH2_BUF_START_ADDR, BCPU2HCPU_MB_CH2_BUF_SIZE, // BCPU->HCPU, Channel 2
    // Patch
    BCPU_PATCH_CODE_START_ADDR, BCPU_PATCH_CODE_SIZE,           // Patch code
    BCPU_PATCH_RAM_START_ADDR, BCPU_PATCH_RAM_SIZE,             // Patch RAM
    BLE_RAM_BASE, BLE_RAM_SIZE,                                 // BCPU Total RAM

    // Flash allocation
    FLASH_TABLE_START_ADDR, FLASH_TABLE_SIZE,
    FLASH_BOOT_PATCH_START_ADDR, FLASH_BOOT_PATCH_SIZE,
    HCPU_FLASH_CODE_START_ADDR, HCPU_FLASH_CODE_SIZE,
    HCPU_FLASH_IMG_START_ADDR, HCPU_FLASH_IMG_SIZE,
    LCPU_FLASH_CODE_START_ADDR, LCPU_FLASH_CODE_SIZE,
};

const char  description[][80] =
{
    "HCPU Boot",
    "HCPU ROM Code",
    "HCPU ROM Data",
    "HCPU RAM",
    "Mailbox HCPU->BCPU channel 2",
    "Mailbox HCPU->LCPU channel 2",
    "Mailbox HCPU->BCPU channel 1",
    "Mailbox HCPU->LCPU channel 1",
    "HCPU RAM Total",

    "LCPU Code",
    "LCPU RAM",
    "Mailbox LCPU->HCPU channel 1",
    "Mailbox LCPU->HCPU channel 2",
    "LCPU Boot",
    "LCPU RAM Total",

    "BCPU Code",
    "BCPU RAM",
    "Mailbox BCPU->HCPU Channel 1",
    "Mailbox BCPU->HCPU Channel 2",
    "BCPU Patch Code",
    "BCPU Patch RAM",
    "BCPU RAM Total",

    "Flash table",
    "Flash HCPU Boot",
    "Flash HCPU Code",
    "Flash HCPU IMG",
    "Flash LCPU Code",
};

int main()
{
    int i;
    unsigned int start, size, end;

    printf("Description, Range, size(hex), size, size(k)\n");
    for (i = 0; i < sizeof(addresses) / (sizeof(unsigned int) * 2); i++)
    {
        start = addresses[i * 2];
        size = addresses[i * 2 + 1];
        end = start + size - 1;
        printf("%s, 0x%08X-0x%08X, 0x%08x,%d,%.2f\n", description[i], start, end, size, size, ((float)size) / 1024.0);
    }
}

