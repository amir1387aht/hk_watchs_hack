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

#include "data_service_subscriber.h"
#include "test_service.h"

static datac_handle_t client_handle;

static int test_service_callback(data_callback_arg_t *arg)
{
    if (MSG_SERVICE_DATA_NTF_IND == arg->msg_id)
    {
        test_service_data_ntf_ind_t *data_ntf_ind;

        RT_ASSERT(arg->data_len == sizeof(test_service_data_ntf_ind_t));
        data_ntf_ind = (test_service_data_ntf_ind_t *)arg->data;

        RT_ASSERT(data_ntf_ind);

        rt_kprintf("data ntf:%d\n", data_ntf_ind->data);
    }
    else if (MSG_SERVICE_TEST_DATA_RSP == arg->msg_id)
    {
        test_service_data_rsp_t *data_rsp;

        RT_ASSERT(arg->data_len == sizeof(test_service_data_rsp_t));
        data_rsp = (test_service_data_rsp_t *)arg->data;

        RT_ASSERT(data_rsp);

        rt_kprintf("data rsp:%d\n", data_rsp->data);
    }
    else if (MSG_SERVICE_TEST_DATA2_IND == arg->msg_id)
    {
        test_service_data2_ind_t *data2_ind;

        RT_ASSERT(arg->data_len == sizeof(test_service_data2_ind_t));
        data2_ind = (test_service_data2_ind_t *)arg->data;

        RT_ASSERT(data2_ind);

        rt_kprintf("data2 ind:%d\n", data2_ind->data);
    }
    else if (MSG_SERVICE_SUBSCRIBE_RSP == arg->msg_id)
    {
        data_subscribe_rsp_t *rsp = (data_subscribe_rsp_t *)arg->data;
        RT_ASSERT(rsp);
        RT_ASSERT(client_handle == rsp->handle);
        rt_kprintf("Subscribe result:%d\n", rsp->result);
    }

    return 0;
}

static int request(int argc, char *argv[])
{
    data_msg_t msg;
    uint8_t *body;
    rt_err_t err;

    body = data_service_init_msg(&msg, MSG_SERVICE_TEST_DATA_REQ, 0);
    err = datac_send_msg(client_handle, &msg);
    RT_ASSERT(RT_EOK == err);

    return 0;
}
MSH_CMD_EXPORT(request, "request data")

int main(void)
{
    client_handle = datac_open();
    RT_ASSERT(DATA_CLIENT_INVALID_HANDLE != client_handle);
    datac_subscribe(client_handle, "test", test_service_callback, 0);

    while (1)
    {
        rt_thread_mdelay(100000);
    }

    return RT_EOK;
}

