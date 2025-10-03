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
#include "data_service_provider.h"
#include "test_service.h"

static datas_handle_t test_service_handle;
static uint32_t test_counter;

static int32_t test_service_msg_handler(datas_handle_t service, data_msg_t *msg);

static data_service_config_t test_service_cb =
{
    .max_client_num = 1,
    .queue = RT_NULL, /* share the same queue of data service process thread */
    .data_filter = NULL,
    .msg_handler = test_service_msg_handler,
};

static int32_t test_service_msg_handler(datas_handle_t service, data_msg_t *msg)
{
    switch (msg->msg_id)
    {
    case MSG_SERVICE_SUBSCRIBE_REQ:
    {
        break;
    }
    case MSG_SERVICE_UNSUBSCRIBE_REQ:
    {
        break;
    }
    case MSG_SERVICE_TEST_DATA_REQ:
    {
        rt_err_t r;
        test_service_data_rsp_t data_rsp;

        rt_kprintf("receive msg DATA_REQ\n");
        test_counter++;
        data_rsp.data = test_counter;
        r = datas_send_response_data(service, msg, sizeof(data_rsp), (uint8_t *)&data_rsp);
        RT_ASSERT(RT_EOK == r);
        break;
    }
    case MSG_SERVICE_DATA_RDY_IND:
    {
        data_rdy_ind_t *data_rdy_ind = (data_rdy_ind_t *)(data_service_get_msg_body(msg));
        int32_t result;
        test_service_data_ntf_ind_t data_ntf_ind;

        RT_ASSERT(data_rdy_ind);

        rt_kprintf("push DATA_NTF_IND to all clients\n");
        data_ntf_ind.data = (uint32_t)data_rdy_ind->data;
        result = datas_push_data_to_client(service, sizeof(data_ntf_ind), (uint8_t *)&data_ntf_ind);
        RT_ASSERT(0 == result);
        break;
    }
    default:
    {
        RT_ASSERT(0);
    }
    }

    return 0;
}

static int test_service_register(void)
{
    /* register test service*/
    test_service_handle = datas_register("test", &test_service_cb);
    RT_ASSERT(test_service_handle);

    return 0;
}
INIT_COMPONENT_EXPORT(test_service_register);

static int trigger(int argc, char *argv[])
{
    rt_err_t err;

    test_counter++;
    err = datas_data_ready(test_service_handle, sizeof(test_counter), (uint8_t *)test_counter);
    RT_ASSERT(RT_EOK == err);

    return 0;
}
MSH_CMD_EXPORT(trigger, "trigger notification to client")

static int trigger2(int argc, char *argv[])
{
    int32_t result;
    test_service_data2_ind_t data2_ind;

    test_counter++;

    data2_ind.data = test_counter;
    result = datas_push_msg_to_client(test_service_handle, MSG_SERVICE_TEST_DATA2_IND, sizeof(data2_ind), (uint8_t *)&data2_ind);
    RT_ASSERT(0 == result);

    return 0;
}
MSH_CMD_EXPORT(trigger2, "trigger data2_ind to client")

int main(void)
{
    uint32_t cnt = 0;
    while (1)
    {
        rt_thread_mdelay(5000);
        if (cnt & 1)
        {
            trigger(0, NULL);
        }
        else
        {
            trigger2(0, NULL);
        }
        cnt++;
    }

    return RT_EOK;
}

