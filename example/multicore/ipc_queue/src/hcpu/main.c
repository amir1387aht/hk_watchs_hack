/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

#define LOG_TAG "main"
#include "log.h"

#include "ipc_queue.h"
#include "ipc_config.h"

static ipc_queue_handle_t test_ipc_queue;
static rt_mailbox_t test_ipc_mb;

int32_t queue_rx_ind(ipc_queue_handle_t handle, size_t size)
{
    rt_err_t err;

    if (RT_NULL != test_ipc_mb)
    {
        err = rt_mb_send(test_ipc_mb, size);
        RT_ASSERT(RT_EOK == err);
    }
    return 0;
}

static int send(int argc, char *argv[])
{
    size_t wr_size;
    size_t msg_size;

    if (argc < 2)
    {
        rt_kprintf("wrong argument\n");
        return -1;
    }
    /* including null terminator */
    msg_size = strlen(argv[1]) + 1;
    wr_size = ipc_queue_write(test_ipc_queue, argv[1], msg_size, 1000);
    RT_ASSERT(wr_size == msg_size);

    return 0;
}
MSH_CMD_EXPORT(send, "send msg to lcpu")

int main(void)
{
    ipc_queue_cfg_t q_cfg;
    rt_uint32_t size;
    rt_err_t result;

    test_ipc_mb = rt_mb_create("test_ipc_mb", 30, RT_IPC_FLAG_FIFO);
    RT_ASSERT(test_ipc_mb);

    q_cfg.qid = IPC_TEST_QUEUE;
    q_cfg.tx_buf_size = HL_IPC_TX_BUF_SIZE;
    q_cfg.tx_buf_addr = HL_IPC_TX_BUF_ADDR;
    q_cfg.tx_buf_addr_alias = HL_IPC_TX_BUF_ADDR_ALIAS;
    q_cfg.rx_buf_addr = HL_IPC_RX_BUF_ADDR;
    q_cfg.rx_ind = queue_rx_ind;
    q_cfg.user_data = 0;

    test_ipc_queue = ipc_queue_init(&q_cfg);
    RT_ASSERT(IPC_QUEUE_INVALID_HANDLE != test_ipc_queue);

    result = ipc_queue_open(test_ipc_queue);
    RT_ASSERT(0 == result);

    while (1)
    {
        result = rt_mb_recv(test_ipc_mb, &size, RT_WAITING_FOREVER);
        RT_ASSERT(RT_EOK == result);
        if (size > 0)
        {
            uint8_t *buf;
            size_t rd_size;
            buf = rt_malloc(size);
            RT_ASSERT(buf);
            rd_size = ipc_queue_read(test_ipc_queue, buf, size);
            RT_ASSERT(rd_size == size);
            rt_kprintf("rx: %s\n", buf);
            rt_free(buf);
        }
    }

    return RT_EOK;
}

