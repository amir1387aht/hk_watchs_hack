/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_MPI_H__
#define __DRV_MPI_H__

#include <rtconfig.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief  Set MPI ALIAS
* @param[in]  addr, mpi base address .
* @param[in]  start, alias start address .
* @param[in]  len, alias length .
* @param[in]  offset, alias offset .
* @retval none
*/
void rt_mpi_set_alias(uint32_t addr, uint32_t start, uint32_t len, uint32_t offset);

/**
* @brief  Set MPI NONCE
* @param[in]  addr, mpi base address .
* @param[in]  start, start address .
* @param[in]  end, end address.
* @param[in]  nonce, data.
* @retval none
*/
void rt_mpi_set_ctr_nonce(uint32_t addr, uint32_t start, uint32_t end, uint8_t *nonce);

/**
* @brief  Set MPI AES mode
* @param[in]  addr, mpi base address .
* @param[in]  aes256, aes mode.
* @retval none
*/
void rt_mpi_enable_aes(uint32_t addr, uint8_t aes256);

/**
* @brief  Disable MPI AES
* @param[in]  addr, mpi base address .
* @retval none
*/
void rt_mpi_disable_aes(uint32_t addr);

/**
* @brief  Enable MPI prefetch mode
* @param[in]  addr, prefetch phy address .
* @param[in]  len, prefetch data length.
* @retval 0 if success
*/
int rt_mpi_enable_prefetch(uint32_t addr, uint32_t length);

/**
* @brief  Disable MPI prefetch mode
* @param[in]  addr, prefetch phy address or MPI based address.
* @retval 0 if success
*/
int rt_mpi_disable_prefetch(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_MPI_H__ */

