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

#include "rpmsg_lite.h"
#include "ipc_config.h"
#include "bf0_mbox_common.h"

static rt_mq_t test_rpmsg_mq;
static struct rpmsg_lite_endpoint *my_ept;
static struct rpmsg_lite_instance *my_rpmsg;

static int32_t rpmsg_rx_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv);

int main(void)
{
    rt_err_t result;
    rpmsg_rx_cb_data_t rx_data;
    int32_t r;
    int32_t timeout;
    rt_base_t tick;

    lcpu_power_on();

    rt_pm_request(PM_SLEEP_MODE_IDLE);

    test_rpmsg_mq = rt_mq_create("test_rpmsg_mq", sizeof(rpmsg_rx_cb_data_t), 30, RT_IPC_FLAG_FIFO);
    RT_ASSERT(test_rpmsg_mq);

    my_rpmsg = rpmsg_lite_master_init((void *)RPMSG_BUF_ADDR_MASTER, RPMSG_BUF_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);
    RT_ASSERT(my_rpmsg);

    my_ept = rpmsg_lite_create_ept(my_rpmsg, MASTER_EPT_ADDR, rpmsg_rx_cb, (void *)2);
    RT_ASSERT(my_ept);

    timeout = 5000;
    while (1)
    {
        tick = rt_tick_get();
        result = rt_mq_recv(test_rpmsg_mq, &rx_data, sizeof(rx_data), timeout);
        if ((RT_EOK == result) && (rx_data.len > 0))
        {
            uint8_t *buf;
            buf = rt_malloc(rx_data.len);
            RT_ASSERT(buf);
            memcpy((void *)buf, (void *)rx_data.data, rx_data.len);
            r = rpmsg_lite_release_rx_buffer(my_rpmsg, rx_data.data);
            RT_ASSERT(RL_SUCCESS == r);
            rt_kprintf("from: %d\n", rx_data.src);
            rt_kprintf("rx: %s\n", buf);
            rt_free(buf);
        }
        tick = rt_tick_get() - tick;
        if (tick < timeout)
        {
            timeout -= tick;
        }
        else
        {
            timeout = 5000;
            r = rpmsg_lite_send(my_rpmsg, my_ept, REMOTE_EPT_ADDR, "hello_from_hcpu", strlen("hello_from_hcpu") + 1, 1000);
            RT_ASSERT(RL_SUCCESS == r);
        }
    }

    return RT_EOK;
}

static int32_t rpmsg_rx_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    rpmsg_rx_cb_data_t msg;

    RL_ASSERT(priv != RL_NULL);

    msg.data = payload;
    msg.len  = payload_len;
    msg.src  = src;
    msg.priv = priv;

    /* if message is successfully added into queue then hold rpmsg buffer */
    if (RT_EOK == rt_mq_send(test_rpmsg_mq, &msg, sizeof(msg)))
    {
        /* hold the rx buffer */
        return RL_HOLD;
    }

    return RL_RELEASE;
}

static int send(int argc, char *argv[])
{
    int32_t r;
    size_t msg_size;

    if (argc < 2)
    {
        rt_kprintf("wrong argument\n");
        return -1;
    }
    /* including null terminator */
    msg_size = strlen(argv[1]) + 1;
    r = rpmsg_lite_send(my_rpmsg, my_ept, REMOTE_EPT_ADDR, argv[1], msg_size, 1000);
    RT_ASSERT(RL_SUCCESS == r);

    rt_pm_release(PM_SLEEP_MODE_IDLE);

    return 0;
}
MSH_CMD_EXPORT(send, "send msg to lcpu")

