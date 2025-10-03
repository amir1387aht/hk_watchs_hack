/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DFU_UART_H
#define __DFU_UART_H



#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t magic;
    uint8_t running_img; // 0 for ota, 1 for hcpu
} dfu_running_img_info;

typedef struct
{
    rt_device_t device;
    rt_mailbox_t to_mb;

    // for assemable data packet
    uint8_t last_command;
    uint8_t is_assemable;
    uint8_t *assemable_data;
    uint16_t assemable_length;
    uint16_t target_length;


    uint8_t state;
    uint8_t no_data;


    uint32_t image_total_length;
    uint32_t image_current_length;
    uint32_t base_addr;
    uint32_t remote_crc;
    uint16_t image_data_index;
    uint32_t single_packet_size;

    uint8_t mode;
} dfu_uart_env_t;

typedef enum
{
    DFU_RUNNING_ON_OTA,
    DFU_RUNNING_ON_HCPU,
} dfu_running_img_t;

typedef enum
{
    DFU_UART_START_REQ = 1,
    DFU_UART_START_RSP,
    DFU_UART_IMAGE_START_REQ,
    DFU_UART_IMAGE_START_RSP,
    DFU_UART_IMAGE_DATA_IND,
    DFU_UART_IMAGE_DATA_CFM,
    DFU_UART_IMAGE_END_REQ,
    DFU_UART_IMAGE_END_RSP,
    DFU_UART_END_REQ,
    DFU_UART_END_RSP,
} dfu_uart_command_t;

typedef enum
{
    DFU_UART_STATE_NONE,
    DFU_UART_STATE_DOWNLOAD_START,
    DFU_UART_STATE_DOWNLOADING,
    DFU_UART_STATE_DOWNLOAD_END,
    DFU_UART_STATE_INSTALL,
    DFU_UART_STATE_FORCE_UPDATE,
} dfu_uart_state_t;

typedef enum
{
    DFU_UART_ERROR_NO_ERROR,
    DFU_UART_ERROR_INDEX_ERROR,
    DFU_UART_ERROR_FLASH_ERASE_ERROR,
    DFU_UART_ERROR_FILE_LENGTH_ERROR,
    DFU_UART_ERROR_PACKET_LENGTH_ERROR,
    DFU_UART_ERROR_CRC_ERROR,
} dfu_uart_error_t;

// save image state position
#define DFU_RUNNING_IMG_STATE_ADDR 0x10032000

#ifdef SF32LB55X
#define DFU_RUNNING_IMG_INFO_MAX_SIZE 8192
#else
#define DFU_RUNNING_IMG_INFO_MAX_SIZE 4096
#endif

// hcpu image addr
#define HCPU2_CODE_START_ADDR 0x10060000

#define SEC_CONFIG_MAGIC    0x53454346

#define SFUART_HEADER_FRONT 0x41554653
#define SFUART_HEADER_REAR 0x5452
#define SFUART_HEADER_FRONT_LEN 4
#define SFUART_HEADER_REAR_LEN 2
#define SFUART_HEADER_LEN 6

#define DFU_UART_HEADER_EX_LEN 10  // header + command + len

#define UART_FORWARD_PORT "uart1"

#define DFU_UART_MAX_BLOCK_SIZE 2048

#define DFU_UART_VERSION 1

typedef struct
{
    uint32_t image_length;
    uint32_t addr;
    uint32_t crc;
    uint16_t data_size;
} dfu_image_start_req_t;

typedef struct
{
    uint16_t command;
    uint16_t length;
    uint16_t result;
} dfu_image_start_rsp_t;

typedef struct
{
    uint16_t index;
    uint8_t packet[0];
} dfu_image_data_t;

typedef struct
{
    uint16_t command;
    uint16_t length;
    uint16_t result;
    uint16_t current_index;
} dfu_image_data_cfm_t;

typedef struct
{
    uint16_t command;
    uint16_t length;
    uint16_t result;
} dfu_image_end_rsp_t;

typedef struct
{
    uint16_t command;
    uint16_t length;
    uint16_t result;
    uint16_t version;
} dfu_start_rsp_t;

typedef struct
{
    uint16_t command;
    uint16_t length;
    uint16_t result;
} dfu_end_rsp_t;

void dfu_uart_reset_handler();

void dfu_uart_dfu_mode_set();

#ifdef __cplusplus
}
#endif
#endif //__DFU_UART_H