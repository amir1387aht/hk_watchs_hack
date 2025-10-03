/*
 * SPDX-FileCopyrightText: 2025-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "bf0_hal.h"

/* Common functions for RT-Thread based platform -----------------------------------------------*/
#include "drv_io.h"
/**
  * @brief  Initialize board default configuration.
  * @param  None
  * @retval None
  */
void HAL_MspInit(void)
{
    //__asm("B .");        /*For debugging purpose*/
    BSP_IO_Init();
}

/* User code start from here --------------------------------------------------------*/
#include "bf0_sibles.h"
#include "bts2_app_inc.h"
#include "bt_connection_manager.h"
#include "ulog.h"

#include "a2dp_relay.h"

#ifdef     AUDIO_USING_MANAGER
    #include "audio_server.h"
#endif

#ifdef ZBT
    #include "zephyr/bluetooth/bluetooth.h"
#endif

#define BT_APP_READY  1

typedef enum
{
    APP_RELAY_IDLE,
    APP_RELAY_ENABLING,
    APP_RELAY_ENABLED,
    APP_RELAY_PAIRED,
} APP_RELAY_STATE_t;

typedef struct
{
    bt_notify_device_mac_t  addr;
    uint8_t is_a2dp_connected;
    uint8_t is_abs_enabled;
    uint8_t role;
    APP_RELAY_STATE_t state;
} bt_app_t;

static bt_app_t g_bt_app_env;
static rt_mailbox_t g_bt_app_mb;

static bt_app_t *bt_app_get_env(void)
{
    return &g_bt_app_env;
}

static void bt_app_set_role(uint8_t role)
{
    g_bt_app_env.role = role;
}

static uint8_t bt_app_get_role(void)
{
    return g_bt_app_env.role;
}

/** Mount file system if using NAND, as BT NVDS is save in file*/
#if defined(BSP_USING_SPI_NAND) && defined(RT_USING_DFS) && !defined(ZBT)
#include "dfs_file.h"
#include "dfs_posix.h"
#include "drv_flash.h"
#define NAND_MTD_NAME    "root"
int mnt_init(void)
{
    //TODO: how to get base address
    register_nand_device(FS_REGION_START_ADDR & (0xFC000000), FS_REGION_START_ADDR - (FS_REGION_START_ADDR & (0xFC000000)), FS_REGION_SIZE, NAND_MTD_NAME);
    if (dfs_mount(NAND_MTD_NAME, "/", "elm", 0, 0) == 0) // fs exist
    {
        rt_kprintf("mount fs on flash to root success\n");
    }
    else
    {
        // auto mkfs, remove it if you want to mkfs manual
        rt_kprintf("mount fs on flash to root fail\n");
        if (dfs_mkfs("elm", NAND_MTD_NAME) == 0)
        {
            rt_kprintf("make elm fs on flash sucess, mount again\n");
            if (dfs_mount(NAND_MTD_NAME, "/", "elm", 0, 0) == 0)
                rt_kprintf("mount fs on flash success\n");
            else
                rt_kprintf("mount to fs on flash fail\n");
        }
        else
            rt_kprintf("dfs_mkfs elm flash fail\n");
    }
    return RT_EOK;
}
INIT_ENV_EXPORT(mnt_init);
#endif

static void app_a2dp_relay_enable(uint8_t role)
{
    uint8_t ret;
    bt_app_t *env = bt_app_get_env();
    if (role == A2DP_RELAY_SRC)
    {
        ret = a2dp_relay_src_enable();
        if (ret == A2DP_RELAY_ERR_NO_ERROR)
            env->state = APP_RELAY_ENABLING;
        LOG_I("src enable %d", ret);
    }
    else if (role == A2DP_RELAY_SINK)
    {
        ret = a2dp_relay_sink_enable();
        if (ret == A2DP_RELAY_ERR_NO_ERROR)
            env->state = APP_RELAY_ENABLING;
        LOG_I("sink enable %d", ret);
    }
    else
        LOG_W("Wronly local role!");

}

static void app_a2dp_relay_disable(uint8_t role)
{
    uint8_t ret;
    if (role == A2DP_RELAY_SRC)
    {
        ret = a2dp_relay_src_disable();
        LOG_I("src disable %d", ret);
    }
    else if (role == A2DP_RELAY_SINK)
    {
        ret = a2dp_relay_sink_disable();
        LOG_I("sink disable %d", ret);
    }
    else
        LOG_W("Wronly local role!");
}

static int bt_app_interface_event_handle(uint16_t type, uint16_t event_id, uint8_t *data, uint16_t data_len)
{
    bt_app_t *env = bt_app_get_env();

    if (type == BT_NOTIFY_COMMON)
    {
        switch (event_id)
        {
        case BT_NOTIFY_COMMON_BT_STACK_READY:
        {
            rt_mb_send(g_bt_app_mb, BT_APP_READY);
        }
        break;
        default:
            break;
        }
    }
    else if (type == BT_NOTIFY_A2DP)
    {
        switch (event_id)
        {
        case BT_NOTIFY_A2DP_PROFILE_CONNECTED:
        {
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            if (profile_info->res == BTS2_SUCC)
            {
                env->addr = profile_info->mac;
                env->is_a2dp_connected = 1;
                // Set to relay src which connect to phone
                bt_app_set_role(A2DP_RELAY_SRC);
            }
            LOG_I("A2DP connected");
        }
        break;
        case BT_NOTIFY_A2DP_PROFILE_DISCONNECTED:
        {
            bt_notify_profile_state_info_t *info = (bt_notify_profile_state_info_t *)data;
            env->is_a2dp_connected = 0;
            LOG_I("A2DP disconnected %d", info->res);
            if (env->state == APP_RELAY_IDLE)
                bt_app_set_role(A2DP_RELAY_SINK);
        }
        break;
        }
    }
    else if (type == BT_NOTIFY_AVRCP)
    {
        switch (event_id)
        {
        case BT_NOTIFY_AVRCP_PROFILE_CONNECTED:
        {
            LOG_I("AVRCP connected");
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            bt_interface_set_avrcp_role_ext(&profile_info->mac, AVRCP_CT);
        }
        break;
        case BT_NOTIFY_AVRCP_PROFILE_DISCONNECTED:
        {
            bt_notify_profile_state_info_t *info = (bt_notify_profile_state_info_t *)data;
            env->is_abs_enabled = 0;
            LOG_I("AVRCP disconnected %d", info->res);
        }
        break;
        case BT_NOTIFY_AVRCP_VOLUME_CHANGED_REGISTER:
        {
            env->is_abs_enabled = 1;
        }
        break;
        case BT_NOTIFY_AVRCP_ABSOLUTE_VOLUME:
        {
            uint8_t *volume = (uint8_t *)data;
            if (env->state == APP_RELAY_PAIRED)
            {
                a2dp_relay_vol_set(*volume);
            }
#ifdef AUDIO_USING_MANAGER
            uint8_t local_vol = bt_interface_avrcp_abs_vol_2_local_vol(*volume, audio_server_get_max_volume());
            audio_server_set_private_volume(AUDIO_TYPE_BT_MUSIC, local_vol);
#endif
        }
        break;
        default:
            break;
        }
    }

    return 0;
}

uint8_t app_a2dp_relay_callback(A2DP_RELAY_EVT_t evt, uint8_t *data, uint16_t len)
{
    bt_app_t *env = bt_app_get_env();
    switch (evt)
    {
    case A2DP_RELAY_EVT_ENABLED:
    {
        uint8_t res = *data;
        if (res == A2DP_RELAY_ERR_NO_ERROR)
        {
            if (env->role == A2DP_RELAY_SINK)
                gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 0, 0);
            env->state = APP_RELAY_ENABLED;
        }
        else
            env->state = APP_RELAY_IDLE;
        break;
    }
    case A2DP_RELAY_EVT_PAIRED:
    {
        uint8_t role = *data;
        if (role != env->role)
        {
            LOG_E("Wrongly role config origin role: %d, paired role %d", env->role, role);
            app_a2dp_relay_disable(role);
        }
        else
        {
            LOG_I("A2DP relay paired! Role is %d", role);
            env->state = APP_RELAY_PAIRED;
        }
        break;
    }
    case A2DP_RELAY_EVT_DISABLED:
    {
        if (env->role == A2DP_RELAY_SINK)
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 1, 1);
        env->state = APP_RELAY_IDLE;
        if (!env->is_a2dp_connected)
            bt_app_set_role(A2DP_RELAY_SINK);
        break;
    }
    default:
        break;
    }

    return 0;
}

static void app_a2dp_relay_init(void)
{
    bt_app_t *env = bt_app_get_env();
    a2dp_relay_src_init(app_a2dp_relay_callback);
    a2dp_relay_sink_init(app_a2dp_relay_callback);
    env->role = A2DP_RELAY_SINK;
}

/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
#ifdef BT_DEVICE_NAME
    static const char *local_name = BT_DEVICE_NAME;
#else
    static const char *local_name = "sifli_music_sink";
#endif

int main(void)
{
    uint32_t value;

    //__asm("B .");
    g_bt_app_mb = rt_mb_create("bt_app", 8, RT_IPC_FLAG_FIFO);
    RT_ASSERT(g_bt_app_mb);

    bt_interface_register_bt_event_notify_callback(bt_app_interface_event_handle);

    // Start BT/BLE stack/profile.
#ifdef ZBT
    bt_enable(NULL);
#else
    sifli_ble_enable();
#endif

    // Wait for stack/profile ready.
    if (RT_EOK == rt_mb_recv(g_bt_app_mb, (rt_uint32_t *)&value, 8000) && value == BT_APP_READY)
    {
        LOG_I("BT/BLE stack and profile ready");
        app_a2dp_relay_init();
        // Update Bluetooth name
        bt_interface_set_local_name(strlen(local_name), (void *)local_name);
    }
    else
        LOG_I("BT/BLE stack and profile init failed");

    while (1)
    {
        rt_thread_mdelay(15000);
    }
    return 0;
}

static void help(void)
{
    LOG_E("Parameters are too less");
}

__ROM_USED void music(int argc, char **argv)
{
    bt_app_t *env = bt_app_get_env();

    if (argc < 2)
        help();
    else
    {
        if (strcmp(argv[1], "c") == 0)
        {
#ifdef BSP_BT_CONNECTION_MANAGER
            bt_cm_delete_bonded_devs();
#endif // BSP_BT_CONNECTION_MANAGER
        }
        else if (strcmp(argv[1], "set_vol") == 0)
        {
            uint8_t max_vol = 15;
#ifdef AUDIO_USING_MANAGER
            max_vol = audio_server_get_max_volume();
#endif // AUDIO_USING_MANAGER
            uint8_t local_vol = atoi(argv[2]);
            uint8_t abs_vol = bt_interface_avrcp_local_vol_2_abs_vol(local_vol, max_vol);
            bt_interface_avrcp_set_absolute_volume_as_ct_role(abs_vol);
#ifdef AUDIO_USING_MANAGER
            // If absolute volume register by peer device, then local volume shall also adjust.
            if (env->is_abs_enabled)
                audio_server_set_private_volume(AUDIO_TYPE_BT_MUSIC, local_vol);
#endif // AUDIO_USING_MANAGER
        }
        else if (strcmp(argv[1], "pair") == 0)
        {
            // according to current role to call sink or source
            if (env->state != APP_RELAY_PAIRED)
                app_a2dp_relay_enable(bt_app_get_role());
            else
                LOG_W("already paired");
        }
        else if (strcmp(argv[1], "set_role") == 0)
        {
            if (env->state == APP_RELAY_IDLE)
            {
                uint8_t role = atoi(argv[2]);
                LOG_I("set role %d", role);
                bt_app_set_role(role);
            }
            else
                LOG_W("state(%d) is not correct", env->state);
        }
        else if (strcmp(argv[1], "get_role") == 0)
        {
            LOG_I("Role is %d", bt_app_get_role());
        }
        else if (strcmp(argv[1], "unpair") == 0)
        {
            app_a2dp_relay_disable(bt_app_get_role());
        }
        else if (strcmp(argv[1], "set_chnl") == 0)
        {
            uint8_t channel = atoi(argv[2]);
            uint8_t ret = a2dp_relay_src_channel_set(channel);
            LOG_I("chan(%d) set : res %d", channel, ret);
        }
    }
}
MSH_CMD_EXPORT(music, music command)

