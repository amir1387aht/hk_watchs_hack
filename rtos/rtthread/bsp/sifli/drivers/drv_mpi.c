/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "board.h"

/** @addtogroup bsp_driver Driver IO
  * @{
  */
#if defined(BSP_USING_MPI)||defined(_SIFLI_DOXYGEN_)

//FLASH_HandleTypeDef mpi_instance[5];

static int rt_mpi_addr2id(uint32_t addr)
{
    // for address range over map, do not process, user need care about it.
    if ((addr >= MPI1_MEM_BASE) && (addr < (MPI1_MEM_BASE + QSPI1_MAX_SIZE)))
        return 1;
    if ((addr >= MPI2_MEM_BASE) && (addr < (MPI2_MEM_BASE + QSPI2_MAX_SIZE)))
        return 2;
#ifdef FLASH3
    if ((addr >= MPI3_MEM_BASE) && (addr < (MPI3_MEM_BASE + QSPI3_MAX_SIZE)))
        return 3;
#endif
#ifdef BSP_ENABLE_QSPI4
    if ((addr >= MPI4_MEM_BASE) && (addr < (MPI4_MEM_BASE + QSPI4_MAX_SIZE)))
        return 4;
#endif
#ifdef FLASH5
    if ((addr >= MPI5_MEM_BASE) && (addr < (MPI5_MEM_BASE + QSPI5_MAX_SIZE)))
        return 5;
#endif
    return 0;
}

static uint32_t rt_mpi_id2base(int id)
{
    uint32_t addr = 0;
    switch (id)
    {
    case 1:
        addr = MPI1_MEM_BASE;
        break;
    case 2:
        addr = MPI2_MEM_BASE;
        break;

#ifdef FLASH3
    case 3:
        addr = MPI3_MEM_BASE;
        break;
#endif
#ifdef BSP_ENABLE_QSPI4
    case 4:
        addr = MPI4_MEM_BASE;
        break;
#endif
#ifdef FLASH5
    case 5:
        addr = MPI5_MEM_BASE;
        break;
#endif
    default:
        addr = 0;
        break;
    }

    return addr;
}

static MPI_TypeDef *rt_mpi_id2handle(int id)
{
    MPI_TypeDef *hmpi = NULL;
    switch (id)
    {
    case 1:
        hmpi = hwp_mpi1;
        break;
    case 2:
        hmpi = hwp_mpi2;
        break;
#ifdef FLASH3
    case 3:
        hmpi = hwp_mpi3;
        break;
#endif
#ifdef BSP_ENABLE_QSPI4
    case 4:
        hmpi = hwp_mpi4;
        break;
#endif
#ifdef FLASH5
    case 5:
        hmpi = hwp_mpi5;
        break;
#endif
    default:
        hmpi = NULL;
        break;
    }

    return hmpi;
}

__HAL_ROM_USED void rt_mpi_set_alias(uint32_t addr, uint32_t start, uint32_t len, uint32_t offset)
{
    FLASH_HandleTypeDef fhandle;

    int id = rt_mpi_addr2id(addr);
    if (id > 0)
    {
        fhandle.base = rt_mpi_id2base(id);
        fhandle.Instance = rt_mpi_id2handle(id);
        HAL_FLASH_ALIAS_CFG(&fhandle, start, len, offset);
    }
}

__HAL_ROM_USED void rt_mpi_set_ctr_nonce(uint32_t addr, uint32_t start, uint32_t end, uint8_t *nonce)
{
    FLASH_HandleTypeDef fhandle;

    int id = rt_mpi_addr2id(addr);
    if (id > 0)
    {
        fhandle.base = rt_mpi_id2base(id);
        fhandle.Instance = rt_mpi_id2handle(id);
        HAL_FLASH_NONCE_CFG(&fhandle, start, end, nonce);
    }
}

__HAL_ROM_USED void rt_mpi_enable_aes(uint32_t addr, uint8_t aes256)
{
    FLASH_HandleTypeDef fhandle;

    int id = rt_mpi_addr2id(addr);
    if (id > 0)
    {
        fhandle.base = rt_mpi_id2base(id);
        fhandle.Instance = rt_mpi_id2handle(id);
        HAL_FLASH_AES_CFG(&fhandle, aes256);
    }
}

__HAL_ROM_USED void rt_mpi_disable_aes(uint32_t addr)
{
    FLASH_HandleTypeDef fhandle;

    int id = rt_mpi_addr2id(addr);
    if (id > 0)
    {
        fhandle.base = rt_mpi_id2base(id);
        fhandle.Instance = rt_mpi_id2handle(id);
        HAL_FLASH_ENABLE_AES(&fhandle, 0);
    }
}

__HAL_ROM_USED int rt_mpi_enable_prefetch(uint32_t addr, uint32_t length)
{
    FLASH_HandleTypeDef fhandle;
    uint32_t start;
    HAL_StatusTypeDef ret = HAL_OK;

    int id = rt_mpi_addr2id(addr);
    if (id > 0)
    {
        fhandle.base = rt_mpi_id2base(id);
        fhandle.Instance = rt_mpi_id2handle(id);
        start = addr - fhandle.base;    // to offset
        ret = HAL_MPI_CFG_PREFETCH(&fhandle, start, length);
        if (ret != HAL_OK)
            return 1;

        ret = HAL_MPI_EN_PREFETCH(&fhandle, 1);
        if (ret != HAL_OK)
            return 1;

        return 0;
    }

    return 1;
}

__HAL_ROM_USED int rt_mpi_disable_prefetch(uint32_t addr)
{
    FLASH_HandleTypeDef fhandle;
    HAL_StatusTypeDef ret = HAL_OK;

    int id = rt_mpi_addr2id(addr);
    if (id > 0)
    {
        fhandle.base = rt_mpi_id2base(id);
        fhandle.Instance = rt_mpi_id2handle(id);

        ret = HAL_MPI_EN_PREFETCH(&fhandle, 0);
        if (ret != HAL_OK)
            return 1;

        return 0;
    }

    return 1;
}

#endif //BSP_USING_MPI

/// @} bsp_driver

