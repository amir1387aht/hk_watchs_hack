/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_common.h"
#include "bts2_app_inc.h"
#include "ble_connection_manager.h"
#include "bt_connection_manager.h"

#define LOG_TAG "spp_app"
#include "log.h"

typedef struct
{
    U16 connected_profile;
    BOOL bt_connected;
    BTS2S_BD_ADDR bd_addr;
} bt_app_t;

static bt_app_t g_bt_app_env;
static rt_mailbox_t g_bt_app_mb;

int bt_app_hci_event_handler(uint16_t event_id, uint8_t *msg)
{
    switch (event_id)
    {
    case DM_EN_ACL_OPENED_IND:
    {
        BTS2S_DM_EN_ACL_OPENED_IND *ind = (BTS2S_DM_EN_ACL_OPENED_IND *)msg;

        LOG_I("[bt_app]link connected COD:%d Incoming:%d res %d\r\n", ind->dev_cls, ind->incoming, ind->st);
        LOG_I("[bt_app]bd addr %x-%x-%x\r\n", ind->bd.nap, ind->bd.uap, ind->bd.lap);

        // Enable first
        if (ind->st != HCI_SUCC)
        {
            LOG_I("[bt_app]acl connect fail!!!!\n");
        }
        else
        {
            memcpy(&g_bt_app_env.bd_addr, &ind->bd, sizeof(BTS2S_BD_ADDR));
            g_bt_app_env.bt_connected = TRUE;
            g_bt_app_env.connected_profile = 0;
        }
        break;
    }
    case DM_ACL_DISC_IND:
    {
        BTS2S_DM_ACL_DISC_IND *ind = (BTS2S_DM_ACL_DISC_IND *)msg;
        LOG_I("[bt_app]link dis-connected %x %d\r\n", ind->hdl, ind->reason);

        g_bt_app_env.bt_connected = FALSE;
        g_bt_app_env.connected_profile = 0;

        g_bt_app_env.bd_addr.lap = CFG_BD_LAP;
        g_bt_app_env.bd_addr.nap = CFG_BD_NAP;
        g_bt_app_env.bd_addr.uap = CFG_BD_UAP;
        break;
    }
    }

    return 0;
}

int bt_app_gap_event_handler(uint16_t event_id, uint8_t *msg)
{
    switch (event_id)
    {
    // Using RD LOCAL NAME CFM as app init completed
    case BTS2MU_GAP_RD_LOCAL_NAME_CFM:
    {
        BTS2S_GAP_RD_LOCAL_NAME_CFM *my_msg = (BTS2S_GAP_RD_LOCAL_NAME_CFM *)msg;

        //!zhengyu:Reading the name is complete, and the customer needs to adapt here
        break;
    }
    case BTS2MU_GAP_SET_LOCAL_NAME_CFM:
    {
        BTS2S_GAP_SET_LOCAL_NAME_CFM *my_msg;

        my_msg = (BTS2S_GAP_SET_LOCAL_NAME_CFM *)msg;

        if (my_msg->res == BTS2_SUCC)
        {
            //!zhengyu:modify the name is complete, and the customer needs to adapt here
        }
        else
        {
            //!zhengyu:modify the name fail
        }
        break;
    }
    case BTS2MU_GAP_RD_LOCAL_BD_ADDR_CFM:
    {
        BTS2S_GAP_RD_LOCAL_BD_ADDR_CFM *my_msg;

        my_msg = (BTS2S_GAP_RD_LOCAL_BD_ADDR_CFM *)msg;

        //!zhengyu:read bt addr is complete, and the customer needs to adapt here

        break;
    }
    case BTS2MU_GAP_RD_COD_CFM:
    {
        break;
    }

    default:
        break;
    }

    return 0;
}


//customers need to implement their own code here
void bt_app_spp_srv_event_handler(uint16_t event_id, uint8_t *msg)
{
    switch (event_id)
    {
    case BTS2MU_SPP_CONN_CFM:
    {
        BTS2S_SPP_CONN_CFM *my_msg = NULL;
        my_msg = (BTS2S_SPP_CONN_CFM *)msg;

        U8 device_id;

        if (my_msg->res == BTS2_SUCC)
        {
            //!Customer adaptation
            device_id = bt_interface_spp_get_device_id_by_addr(&my_msg->bd);
            if (device_id != 0xff)
            {
                INFO_TRACE("%s,%d\n", __func__, __LINE__);
            }
        }
        else
        {
            //!Customer adaptation
            INFO_TRACE("%s,%d\n", __func__, __LINE__);
        }
        break;
    }
    case BTS2MU_SPP_DATA_IND:
    {
        BTS2S_SPP_DATA_IND *my_msg;

        my_msg = (BTS2S_SPP_DATA_IND *) msg;

        //!Customer adaptation
        INFO_TRACE("%s,%d\n", __func__, __LINE__);
        break;
    }
    case BTS2MU_SPP_DATA_CFM:
    {
        BTS2S_SPP_DATA_CFM *my_msg;
        my_msg = (BTS2S_SPP_DATA_CFM *) msg;

        //!Customer adaptation
        INFO_TRACE("%s,%d\n", __func__, __LINE__);
        break;
    }
    case BTS2MU_SPP_DISC_CFM:
    {
        BTS2S_SPP_DISC_IND *my_msg;

        my_msg = (BTS2S_SPP_DISC_IND *) msg;

        //!Customer adaptation
        INFO_TRACE("%s,%d\n", __func__, __LINE__);
        break;
    }
    default:
    {
        INFO_TRACE("<< Unhandled message %x in SPP\n", event_id);
        break;
    }
    }
}


int bt_app_event_hdl(U16 type, U16 event_id, uint8_t *msg, uint32_t context)
{
    if (type == BTS2M_HCI_CMD)
    {
        bt_app_hci_event_handler(event_id, msg);
    }
    else if (type == BTS2M_GAP)
    {
        bt_app_gap_event_handler(event_id, msg);
    }
#ifdef CFG_SPP_SRV
    else if (type == BTS2M_SPP_SRV)
    {
        bt_app_spp_srv_event_handler(event_id, msg);
    }
#endif
    return 0;

}
BT_EVENT_REGISTER_LOW(bt_app_event_hdl, NULL);


// SDK会调用该函数，User只需要在自己工程实现该函数并返回产品定义的COD
uint32_t bt_get_class_of_device(void)
{
    return 0x040680;
}

void bt_sc_io_capability_rsp(BTS2S_BD_ADDR *bd)
{
    bt_io_capability_rsp(bd, IO_CAPABILITY_NO_INPUT_NO_OUTPUT, FALSE, TRUE);
}


void spp_init()
{
    hcia_set_default_pscan_efficient(1);
    hcia_set_default_iscan_efficient(1);
    bt_cm_set_profile_target(0, BT_SLAVE_ROLE, 0);
    bt_interface_acl_accept_role_set(HCI_MASTER);
    bt_interface_set_linkpolicy(1, 1);
    bt_interface_set_sniff_enable(FALSE);
}


//!zhengyu:5.uget SDK release version
version_t bt_get_current_SDK_version(void)
{
    version_t version = {0xff, 0xff, 0xffff};

#ifdef SIFLI_VERSION
    version.version_x = (SIFLI_VERSION >> 24) & 0xff;
    version.version_y = ((SIFLI_VERSION & 0xFF0000) >> 16) & 0xff;
    version.version_z = SIFLI_VERSION & 0xFFFF;
#endif
//!zhengyu:Version form is version_x.version_y.version_z
    return version;
}

