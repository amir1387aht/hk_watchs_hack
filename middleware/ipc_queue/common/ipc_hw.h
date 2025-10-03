/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IPC_HW_H
#define IPC_HW_H
#include <stdbool.h>
#include <stdint.h>
#include "sf_type.h"
#include "ipc_hw_port.h"
#include "ipc_os_port.h"

/**
 ****************************************************************************************
* @addtogroup ipc IPC Library
* @ingroup middleware
* @brief Sifli ipc library interface
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

/** hwmailbox channel configuration */
typedef struct
{
    MAILBOX_HandleTypeDef handle;            /**< hwmailbox handle */
    uint8_t core;                            /**< which core the hwmailbox instance belongs to*/
    IRQn_Type irqn;                          /**< IRQ number by rx side, -1 for tx side  */
} hwmailbox_ch_cfg_t;

typedef uint32_t (*ipc_hw_addr_conv_t)(uint32_t addr);

/** IPC bidirectional channel configuration */
typedef struct
{
    ipc_hw_addr_conv_t addr_conv;           /**< translate tx addrss to address from the view from rx side */
    hwmailbox_ch_cfg_t rx;                  /**< rx channel */
    hwmailbox_ch_cfg_t tx;                  /**< tx channel */
} ipc_ch_cfg_t;

#if IPC_HW_QUEUE_NUM > 32
#error "too large hw queue number"
#endif

typedef struct
{
    uint32_t act_bitmap;
    uint32_t user_data[IPC_HW_QUEUE_NUM];
} ipc_ch_dyn_data_t;

typedef struct
{
    ipc_ch_cfg_t cfg;
    ipc_ch_dyn_data_t data;
} ipc_hw_ch_t;

/** IPC hw configuration */
typedef struct
{
    //uint8_t ch_num;                         /**< number of channels */
    //uint8_t q_num;                          /**< number of queues in one channel */
    ipc_hw_ch_t ch[IPC_HW_CH_NUM];
} ipc_hw_t;

typedef struct
{
    uint8_t ch_id;
    uint8_t q_idx;
} ipc_hw_q_handle_t;

extern ipc_hw_t ipc_hw_obj;

/** @brief Enable interrupt, afterwards the other side can receive interrupt even if it has not called ipc_hw_enable_interrupt.
 *
 *  Unmask the interrupt for tx direction, i.e. user could then trigger interrupt.
 *  Enable the NVIC IRQ for rx direction, i.e. interrupt could be received then.
 *  As all mailbox channels share one NVIC IRQ, interrupt for a channel might be triggerd by receiver
 *  side before it's enabled by ipc_hw_enable_interrupt. Then this interrupt would be lost.
 *
 * @param[out] hw_q_handle hw queue handle
 * @param[in] qid queue id, numbering across all queues in all channels
 * @param[in] user_data user data
 * @retval status, 0: success, otherwise: fail
 *
 */
int32_t ipc_hw_enable_interrupt(ipc_hw_q_handle_t *hw_q_handle, uint8_t qid, uint32_t user_data);

/** @brief New implementation for enabling interrupt to solve interrupt loss issue.
 *
 *  Unmask the interrupt and enable NVIC IRQ for rx direction, i.e. user could then receive interrupt.
 *
 * @param[out] hw_q_handle hw queue handle
 * @param[in] qid queue id, numbering across all queues in all channels
 * @param[in] user_data user data
 * @retval status, 0: success, otherwise: fail
 *
 */
int32_t ipc_hw_enable_interrupt2(ipc_hw_q_handle_t *hw_q_handle, uint8_t qid, uint32_t user_data);

/** @brief disable interrupt
 *
 * Mask interrupt for tx direction and disable NVIC IRQ if no rx channel is enabled
 *
 * @param[in] hw_q_handle hw queue handle
 * @retval status, 0: success, otherwise: fail
 *
 */
int32_t ipc_hw_disable_interrupt(ipc_hw_q_handle_t *hw_q_handle);

/** @brief New implementation for disable interrupt
 *
 * Mask interrupt for rx direction and disable NVIC IRQ if no rx channel is enabled
 *
 * @param[in] hw_q_handle hw queue handle
 * @retval status, 0: success, otherwise: fail
 *
 */
int32_t ipc_hw_disable_interrupt2(ipc_hw_q_handle_t *hw_q_handle);

/** @brief trigger mailbox interrupt
 *
 * @param[in] hw_q_handle hw queue handle
 *
 */
void ipc_hw_trigger_interrupt(ipc_hw_q_handle_t *hw_q_handle);

/** @brief check whether interrupt has been cleared
 *
 * @param[in] hw_q_handle hw queue handle
 *
 */
int32_t ipc_hw_check_interrupt(ipc_hw_q_handle_t *hw_q_handle);

void ipc_queue_data_ind(uint32_t user_data);

/// @}  file

#ifdef __cplusplus
}
#endif

/// @} button
#endif

