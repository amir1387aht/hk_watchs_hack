/*
 * SPDX-FileCopyrightText: 2021-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include "data_service_subscriber.h"
#ifdef BLUETOOTH
    #include "ancs_service.h"
#endif /* BLUETOOTH */
#include "flashdb.h"
#include "drv_flash.h"
#ifdef BLUETOOTH
    #include "bf0_ble_gap.h"
    #include "bf0_sibles.h"
    #include "bf0_sibles_advertising.h"
#endif /* BLUETOOTH */

#define LOG_TAG "pm"
#include "log.h"

static void gpio_wakeup_handler(void *args)
{
    rt_kprintf("gpio_wakeup_handler!\n");
}
static void app_wakeup(void)
{
    uint8_t pin;
#if defined(SF32LB55X)

#if defined(BSP_USING_BOARD_EH_LB557)
    HAL_PIN_Set(PAD_PA80, GPIO_A80, PIN_PULLUP, 1); //PA80 #WKUP_A3

    HAL_StatusTypeDef status = HAL_HPAON_EnableWakeupSrc(HPAON_WAKEUP_SRC_PIN3, AON_PIN_MODE_LOW); //Enable #WKUP_A3(PA80)

    rt_pin_attach_irq(80, PIN_IRQ_MODE_RISING_FALLING, (void *) gpio_wakeup_handler,
                      (void *)(rt_uint32_t) 80);
    rt_pin_irq_enable(80, 1);

#endif
#elif defined(SF32LB56X)
#define WAKE_KEY (96+32) //PB32 #WKUP_PIN0
    HAL_PIN_Set(PAD_PB32, GPIO_B32, PIN_PULLDOWN, 0);       // PWR_KEY
    HAL_StatusTypeDef status = HAL_HPAON_EnableWakeupSrc(HPAON_WAKEUP_SRC_PIN0, AON_PIN_MODE_POS_EDGE); //Enable #WKUP_PIN0 (PB32)

    rt_pin_mode(WAKE_KEY, PIN_MODE_INPUT);
    rt_pin_attach_irq(WAKE_KEY, PIN_IRQ_MODE_RISING, (void *) gpio_wakeup_handler,
                      (void *)(rt_uint32_t) WAKE_KEY); //PB32 GPIO interrupt
    rt_pin_irq_enable(WAKE_KEY, 1);
    rt_kprintf("SF32LB56X WER:0x%x,WSR:0x%x,CR1:0x%x\n", hwp_hpsys_aon->WER, hwp_hpsys_aon->WSR, hwp_hpsys_aon->CR1);

#elif defined(SF32LB52X)

    int8_t wakeup_pin;
    HAL_PIN_Set(PAD_PA34, GPIO_A34, PIN_PULLDOWN, 1); //set PA34 to GPIO funtion

    HAL_HPAON_EnableWakeupSrc(HPAON_WAKEUP_SRC_PIN10, AON_PIN_MODE_POS_EDGE); //Enable #WKUP_PIN10 (PA34)

    rt_pin_mode(BSP_KEY1_PIN, PIN_MODE_INPUT);

    rt_pin_attach_irq(BSP_KEY1_PIN, PIN_IRQ_MODE_RISING, (void *) gpio_wakeup_handler,
                      (void *)(rt_uint32_t) BSP_KEY1_PIN); //PA34 GPIO interrupt
    rt_pin_irq_enable(BSP_KEY1_PIN, 1);

#elif defined(SF32LB58X)
#define WAKE_KEY (96+54) //PB54 #WKUP_PIN0

    HAL_PIN_Set(PAD_PB54, GPIO_B54, PIN_NOPULL, 0);
    HAL_StatusTypeDef status = HAL_HPAON_EnableWakeupSrc(HPAON_WAKEUP_SRC_PIN0, AON_PIN_MODE_HIGH); //enable #WKUP_PIN0 (PB54)

    rt_pin_mode(WAKE_KEY, PIN_MODE_INPUT);
    rt_pin_attach_irq(WAKE_KEY, PIN_IRQ_MODE_FALLING, (void *) gpio_wakeup_handler,
                      (void *)(rt_uint32_t) WAKE_KEY); //PB54 GPIO interrupt
    rt_pin_irq_enable(WAKE_KEY, 1);
    rt_kprintf("SF32LB58X AON CR1:0x%x,CR2:0x%x,WER:0x%x\n", hwp_hpsys_aon->CR1, hwp_hpsys_aon->CR2, hwp_hpsys_aon->WER);

#else

#endif

}

void rc10k_timeout_handler(void *parameter)
{
    HAL_RC_CAL_update_reference_cycle_on_48M(LXT_LP_CYCLE);
}
#if defined(SF32LB52X)||defined(SF32LB58X)
    HAL_RAM_RET_CODE_SECT(sleep,   int sleep(int argc, char **argv))
#else
    int sleep(int argc, char **argv)
#endif
{
    char i;
    if (argc > 1)
    {
        if (strcmp("standby", argv[1]) == 0)
        {
            rt_kprintf("sleep on\r\n");
            rt_pm_release(PM_SLEEP_MODE_IDLE);
        }
        else if (strcmp("off", argv[1]) == 0)
        {
            rt_kprintf("sleep off\r\n");
            rt_pm_request(PM_SLEEP_MODE_IDLE);
        }
        else if (strcmp("down", argv[1]) == 0)
        {
#if defined(SF32LB55X)
            rt_kprintf("SF32LB55X entry_hibernate\n");
            HAL_PIN_Set(PAD_PB48, GPIO_B48, PIN_NOPULL, 0);
            HAL_PMU_EnablePinWakeup(5, AON_PIN_MODE_NEG_EDGE); //PB48 #WKUP_PIN5
            rt_kprintf("SF32LB55X CR:0x%x,WER:0x%x\n", hwp_pmuc->CR, hwp_pmuc->WER);
            rt_hw_interrupt_disable();
            HAL_PMU_EnterHibernate();
            while (1);
#elif defined(SF32LB52X)
            rt_kprintf("SF32LB52X entry_hibernate\n");
            HAL_PMU_SelectWakeupPin(0, HAL_HPAON_QueryWakeupPin(hwp_gpio1, BSP_KEY1_PIN)); //select PA34 to wake_pin0
            HAL_PMU_EnablePinWakeup(0, AON_PIN_MODE_HIGH);  //enable wake_pin0
            hwp_pmuc->WKUP_CNT = 0x50005;    //31-16bit:config PIN1 wake CNT , 15-0bit:PIN0 wake CNT
            rt_kprintf("SF32LB52X CR:0x%x,WER:0x%x\n", hwp_pmuc->CR, hwp_pmuc->WER);
            rt_hw_interrupt_disable();
            HAL_PMU_ConfigPeriLdo(PMUC_PERI_LDO_EN_VDD33_LDO3_Pos, false, false);
            HAL_PMU_ConfigPeriLdo(PMUC_PERI_LDO_EN_VDD33_LDO2_Pos, false, false);
            HAL_PMU_ConfigPeriLdo(PMU_PERI_LDO_1V8, false, false);
            HAL_PMU_EnterHibernate();
            while (1);
#elif defined(SF32LB58X)
            rt_kprintf("SF32LB58X entry_hibernate\n");

            HAL_PIN_Set(PAD_PB54, GPIO_B54, PIN_PULLDOWN, 0);  //Set PB54(#WKUP_PIN0) to GPIO and pulldown
            HAL_PMU_SelectWakeupPin(0, HAL_HPAON_QueryWakeupPin(hwp_gpio2, 54)); //select PB54 to pin0
            HAL_PMU_EnablePinWakeup(0, AON_PIN_MODE_HIGH);  //enable wake_pin0

            rt_kprintf("SF32LB5XX CR:0x%x,WER:0x%x,WKUP_CNT:0x%x,PBR0R:0x%x\n", hwp_pmuc->CR, hwp_pmuc->WER, hwp_pmuc->WKUP_CNT, hwp_rtc->PBR0R);
            rt_hw_interrupt_disable();
            HAL_PBR0_FORCE1_DISABLE();
            //HAL_PIN_Set(PAD_PBR0,PBR_GPO, PIN_NOPULL, 0);
            HAL_PBR_ConfigMode(0, 0); //set PBR0 input
            //HAL_PBR_ConfigMode(0, 1); //set PBR0 output
            //HAL_PBR_WritePin(0, 0); //set PBR0 low
            rt_kprintf("SF32LB58X PBR0R:0x%x\n", hwp_rtc->PBR0R);
            HAL_PMU_EnterHibernate();
            while (1);
#else
            rt_kprintf("DO NOT support hibernate\n");
#endif
        }
        else
        {
            rt_kprintf("sleep cmd err\r\n");
        }
    }
    return 0;
}

MSH_CMD_EXPORT(sleep, forward sleep command); /* 导出到 msh 命令列表中 */

#ifdef BLUETOOTH
enum ble_app_att_list
{
    BLE_APP_SVC,
    BLE_APP_CHAR,
    BLE_APP_CHAR_VALUE,
    BLE_APP_CLIENT_CHAR_CONFIG_DESCRIPTOR,
    BLE_APP_ATT_NB
};

#define app_svc_uuid { \
    0x73, 0x69, 0x66, 0x6c, \
    0x69, 0x5f, 0x61, 0x70, \
    0x70, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00 \
};

#define app_chara_uuid { \
    0x73, 0x69, 0x66, 0x6c, \
    0x69, 0x5f, 0x61, 0x70, \
    0x70, 0x01, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00 \
}

#define SERIAL_UUID_16(x) {((uint8_t)(x&0xff)),((uint8_t)(x>>8))}
/* 24 * 1.25 = 30ms */
#define BLE_APP_HIGH_PERFORMANCE_INTERVAL (24)
#define BLE_APP_TIMEOUT_INTERVAL (5000)

typedef struct
{
    uint8_t is_power_on;
    uint8_t conn_idx;
    uint8_t is_bg_adv_on;
    struct
    {
        bd_addr_t peer_addr;
        uint16_t mtu;
        uint16_t conn_interval;
        uint8_t peer_addr_type;
    } conn_para;
    struct
    {
        sibles_hdl srv_handle;
        uint32_t data;
        uint8_t is_config_on;
    } data;
    rt_timer_t time_handle;
    rt_mailbox_t mb_handle;
#ifdef SF32LB52X
    rt_timer_t rc10k_time_handle;
#endif

} app_env_t;

static app_env_t g_app_env;
static rt_mailbox_t g_app_mb;

static uint8_t g_app_svc[ATT_UUID_128_LEN] = app_svc_uuid;

struct attm_desc_128 app_att_db[] =
{
    [BLE_APP_SVC] = {SERIAL_UUID_16(ATT_DECL_PRIMARY_SERVICE), PERM(RD, ENABLE), 0, 0},
    [BLE_APP_CHAR] = {SERIAL_UUID_16(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    [BLE_APP_CHAR_VALUE] = {
        /* The permissions are for: 1.Allowed read, write req, write command and notification.
                                    2.Write requires Unauthenticated link
           The ext_perm are for: 1. Support 128bit UUID. 2. Read will involve callback. */
        app_chara_uuid, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE) | PERM(WRITE_COMMAND, ENABLE) | PERM(NTF, ENABLE) |
        PERM(WP, UNAUTH), PERM(UUID_LEN, UUID_128) | PERM(RI, ENABLE), 1024
    },
    [BLE_APP_CLIENT_CHAR_CONFIG_DESCRIPTOR] = {
        SERIAL_UUID_16(ATT_DESC_CLIENT_CHAR_CFG), PERM(RD, ENABLE) | PERM(WRITE_REQ,
                ENABLE) | PERM(WP, UNAUTH), PERM(RI, ENABLE), 2
    },

};

static void ble_write_to_remote(void *param);

static app_env_t *ble_app_get_env(void)
{
    return &g_app_env;
}

#ifdef APP_ENABLE_BG_ADV
SIBLES_ADVERTISING_CONTEXT_DECLAR(g_app_advertising_bg_context);

static uint8_t ble_app_background_advertising_event(uint8_t event, void *context, void *data)
{
    switch (event)
    {
    case SIBLES_ADV_EVT_ADV_STARTED:
    {
        sibles_adv_evt_startted_t *evt = (sibles_adv_evt_startted_t *)data;
        LOG_I("Broadcast ADV start resutl %d, mode %d\r\n", evt->status, evt->adv_mode);
        break;
    }
    case SIBLES_ADV_EVT_ADV_STOPPED:
    {
        sibles_adv_evt_stopped_t *evt = (sibles_adv_evt_stopped_t *)data;
        LOG_I("Broadcast ADV stopped reason %d, mode %d\r\n", evt->reason, evt->adv_mode);
        break;
    }
    default:
        break;
    }
    return 0;
}

/* Enable advertise via advertising service. */
static void ble_app_bg_advertising_start(void)
{
    sibles_advertising_para_t para = {0};
    uint8_t ret;

    char local_name[] = "SIFLI_BG_INFO";
    uint8_t manu_additnal_data[] = {0x20, 0xC4, 0x00, 0x91};
    uint16_t manu_company_id = 0x01;

    para.own_addr_type = GAPM_GEN_NON_RSLV_ADDR;
    para.config.adv_mode = SIBLES_ADV_BROADCAST_MODE;
    /* Keep advertising till disable it or connected. */
    para.config.mode_config.broadcast_config.duration = 0x0;
    para.config.mode_config.broadcast_config.interval = 0x140;
    para.config.max_tx_pwr = 0x7F;
    /* Advertising will re-start after disconnected. */
    para.config.is_auto_restart = 1;
    /* Scan rsp data is same as advertising data. */
    para.config.is_rsp_data_duplicate = 1;

    /* Prepare name filed .*/
    para.adv_data.completed_name = rt_malloc(rt_strlen(local_name) + sizeof(sibles_adv_type_name_t));
    para.adv_data.completed_name->name_len = rt_strlen(local_name);
    rt_memcpy(para.adv_data.completed_name->name, local_name, para.adv_data.completed_name->name_len);

    /* Prepare manufacturer filed .*/
    para.adv_data.manufacturer_data = rt_malloc(sizeof(sibles_adv_type_manufacturer_data_t) + sizeof(manu_additnal_data));
    para.adv_data.manufacturer_data->company_id = manu_company_id;
    para.adv_data.manufacturer_data->data_len = sizeof(manu_additnal_data);
    rt_memcpy(para.adv_data.manufacturer_data->additional_data, manu_additnal_data, sizeof(manu_additnal_data));

    para.evt_handler = ble_app_background_advertising_event;

    ret = sibles_advertising_init(g_app_advertising_bg_context, &para);
    if (ret == SIBLES_ADV_NO_ERR)
    {
        sibles_advertising_start(g_app_advertising_bg_context);
    }

    rt_free(para.adv_data.completed_name);
    rt_free(para.adv_data.manufacturer_data);
}
#endif // APP_ENABLE_BG_ADV

SIBLES_ADVERTISING_CONTEXT_DECLAR(g_app_advertising_context);

static uint8_t ble_app_advertising_event(uint8_t event, void *context, void *data)
{
    app_env_t *env = ble_app_get_env();

    switch (event)
    {
    case SIBLES_ADV_EVT_ADV_STARTED:
    {
        sibles_adv_evt_startted_t *evt = (sibles_adv_evt_startted_t *)data;
#ifdef APP_ENABLE_BG_ADV
        if (!env->is_bg_adv_on)
        {
            env->is_bg_adv_on = 1;
            ble_app_bg_advertising_start();
        }
#endif // APP_ENABLE_BG_ADV
        LOG_I("ADV start resutl %d, mode %d\r\n", evt->status, evt->adv_mode);
        break;
    }
    case SIBLES_ADV_EVT_ADV_STOPPED:
    {
        sibles_adv_evt_stopped_t *evt = (sibles_adv_evt_stopped_t *)data;
        LOG_I("ADV stopped reason %d, mode %d\r\n", evt->reason, evt->adv_mode);
        break;
    }
    default:
        break;
    }
    return 0;
}

#define DEFAULT_LOCAL_NAME "SIFLI_APP"
/* Enable advertise via advertising service. */
static void ble_app_advertising_start(void)
{
    sibles_advertising_para_t para = {0};
    uint8_t ret;

    char local_name[31] = {0};
    uint8_t manu_additnal_data[] = {0x20, 0xC4, 0x00, 0x91};
    uint16_t manu_company_id = 0x01;
    bd_addr_t addr;
    ret = ble_get_public_address(&addr);
    if (ret == HL_ERR_NO_ERROR)
        rt_snprintf(local_name, 31, "SIFLI_APP-%x-%x-%x-%x-%x-%x", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
    else
        memcpy(local_name, DEFAULT_LOCAL_NAME, sizeof(DEFAULT_LOCAL_NAME));

    ble_gap_dev_name_t *dev_name = malloc(sizeof(ble_gap_dev_name_t) + strlen(local_name));
    dev_name->len = strlen(local_name);
    memcpy(dev_name->name, local_name, dev_name->len);
    ble_gap_set_dev_name(dev_name);
    free(dev_name);

    para.own_addr_type = GAPM_STATIC_ADDR;
    para.config.adv_mode = SIBLES_ADV_CONNECT_MODE;
    /* Keep advertising till disable it or connected. */
    para.config.mode_config.conn_config.duration = 0x0;
    para.config.mode_config.conn_config.interval = 0x140;
    para.config.max_tx_pwr = 0x7F;
    /* Advertising will re-start after disconnected. */
    para.config.is_auto_restart = 1;
    /* Scan rsp data is same as advertising data. */
    //para.config.is_rsp_data_duplicate = 1;

    /* Prepare name filed. Due to name is too long to put adv data, put it to rsp data.*/
    para.rsp_data.completed_name = rt_malloc(rt_strlen(local_name) + sizeof(sibles_adv_type_name_t));
    para.rsp_data.completed_name->name_len = rt_strlen(local_name);
    rt_memcpy(para.rsp_data.completed_name->name, local_name, para.rsp_data.completed_name->name_len);

    /* Prepare manufacturer filed .*/
    para.adv_data.manufacturer_data = rt_malloc(sizeof(sibles_adv_type_manufacturer_data_t) + sizeof(manu_additnal_data));
    para.adv_data.manufacturer_data->company_id = manu_company_id;
    para.adv_data.manufacturer_data->data_len = sizeof(manu_additnal_data);
    rt_memcpy(para.adv_data.manufacturer_data->additional_data, manu_additnal_data, sizeof(manu_additnal_data));

    para.evt_handler = ble_app_advertising_event;

    ret = sibles_advertising_init(g_app_advertising_context, &para);
    if (ret == SIBLES_ADV_NO_ERR)
    {
        sibles_advertising_start(g_app_advertising_context);
    }

    rt_free(para.rsp_data.completed_name);
    rt_free(para.adv_data.manufacturer_data);
}

uint8_t *ble_app_gatts_get_cbk(uint8_t conn_idx, uint8_t idx, uint16_t *len)
{
    uint8_t *ret_val = NULL;
    app_env_t *env = ble_app_get_env();
    *len = 0;
    switch (idx)
    {
    case BLE_APP_CHAR_VALUE:
    {
        ret_val = (uint8_t *)&env->data.data;
        *len = 4;
        break;
    }
    default:
        break;
    }
    return ret_val;
}

static void ble_app_start_send_thread(uint32_t param)
{
    rt_thread_t tid;
    tid = rt_thread_create("ble_send_example", ble_write_to_remote, (void *)(uint32_t)param, 1024, RT_THREAD_PRIORITY_LOW, 10);
    rt_thread_startup(tid);

}
uint8_t ble_app_gatts_set_cbk(uint8_t conn_idx, sibles_set_cbk_t *para)
{
    app_env_t *env = ble_app_get_env();
    switch (para->idx)
    {
    case BLE_APP_CHAR_VALUE:
    {
        uint32_t old_val = env->data.data;
        if (para->len <= 4)
            memcpy(&env->data.data, para->value, para->len);
        LOG_I("Updated app value from:%d to:%d", old_val, env->data.data);
        if ((env->data.data & 0xFF) == 0x11)
        {
            LOG_I("example of ble notification %d", env->data.data);
            ble_app_start_send_thread(env->data.data);
        }
        break;
    }
    case BLE_APP_CLIENT_CHAR_CONFIG_DESCRIPTOR:
    {
        env->data.is_config_on = *(para->value);
        LOG_I("CCCD %d", env->data.is_config_on);
        if (env->data.is_config_on)
            rt_timer_start(env->time_handle);
        else
            rt_timer_stop(env->time_handle);
        break;
    }
    default:
        break;
    }
    return 0;
}
/**
 * 1. Example of use notification to write a large amount of data to remote.
 * 2. This write function should not call in original ble task, should put into a low priority
 * 3. sibles_write_value_with_rsp can also use if indication permission is enabled
 * 4. except using rt_thread_mdelay to wait and retry, can also write next message
 * after receiving SIBLES_WRITE_VALUE_RSP message
 * 5. may adjust delay times and retry times if data is more larger.
 */
static void ble_write_to_remote(void *param)
{
    app_env_t *env = ble_app_get_env();
    uint32_t para = (uint32_t)param;
    uint8_t idx = para & 0xFF;
    // we will have 20 sets of data
    int data_count = (para & 0xFFFF0000) >> 16;
    int data_length = (para & 0xFF00) >> 8;

    // build up datas
    uint8_t *raw_data = malloc(data_length);

    uint8_t data_index = 0;

    while (data_count > 0)
    {
        // here is notify example
        sibles_value_t value;
        value.hdl = env->data.srv_handle;
        value.idx = BLE_APP_CHAR_VALUE;
        value.len = data_length;
        value.value = raw_data;
        memset(raw_data, data_index, data_length);

        int ret;
        ret = sibles_write_value(env->conn_idx, &value);

        if (ret == data_length)
        {
            // write success
            data_count--;
            data_index++;
        }
        else if (ret == 0)
        {
            // tx queue is full, wait and retry
            int retry = 20;
            while (retry > 0)
            {
                retry--;
                rt_thread_mdelay(50);
                ret = sibles_write_value(env->conn_idx, &value);
                if (ret == data_length)
                {
                    LOG_I("send retry success");
                    data_count--;
                    data_index++;
                    break;
                }
            }
        }
    }
}

static void ble_app_service_init(void)
{
    app_env_t *env = ble_app_get_env();
    sibles_register_svc_128_t svc;

    svc.att_db = (struct attm_desc_128 *)&app_att_db;
    svc.num_entry = BLE_APP_ATT_NB;
    /* Service security level to control all characteristic. */
    svc.sec_lvl = PERM(SVC_AUTH, NO_AUTH) | PERM(SVC_UUID_LEN, UUID_128);
    svc.uuid = g_app_svc;
    /* Reigster GATT service and related callback for next response. */
    env->data.srv_handle = sibles_register_svc_128(&svc);
    if (env->data.srv_handle)
        sibles_register_cbk(env->data.srv_handle, ble_app_gatts_get_cbk, ble_app_gatts_set_cbk);
}

int ble_app_event_handler(uint16_t event_id, uint8_t *data, uint16_t len, uint32_t context)
{
    app_env_t *env = ble_app_get_env();
    switch (event_id)
    {
    case BLE_POWER_ON_IND:
    {
        /* Handle in own thread to avoid conflict */
        if (env->mb_handle)
            rt_mb_send(env->mb_handle, BLE_POWER_ON_IND);
        break;
    }
    case BLE_GAP_CONNECTED_IND:
    {
        ble_gap_connect_ind_t *ind = (ble_gap_connect_ind_t *)data;
        env->conn_idx = ind->conn_idx;
        env->conn_para.conn_interval = ind->con_interval;
        env->conn_para.peer_addr_type = ind->peer_addr_type;
        env->conn_para.peer_addr = ind->peer_addr;
        if (ind->role == 0)
            LOG_E("Peripheral should be slave!!!");

        LOG_I("Peer device(%x-%x-%x-%x-%x-%x) connected", env->conn_para.peer_addr.addr[5],
              env->conn_para.peer_addr.addr[4],
              env->conn_para.peer_addr.addr[3],
              env->conn_para.peer_addr.addr[2],
              env->conn_para.peer_addr.addr[1],
              env->conn_para.peer_addr.addr[0]);

        break;
    }
    case BLE_GAP_UPDATE_CONN_PARAM_IND:
    {
        ble_gap_update_conn_param_ind_t *ind = (ble_gap_update_conn_param_ind_t *)data;
        env->conn_para.conn_interval = ind->con_interval;
        LOG_I("Updated connection interval :%d", ind->con_interval);
        break;
    }
    case SIBLES_MTU_EXCHANGE_IND:
    {
        /* Negotiated MTU. */
        sibles_mtu_exchange_ind_t *ind = (sibles_mtu_exchange_ind_t *)data;
        env->conn_para.mtu = ind->mtu;
        LOG_I("Exchanged MTU size: %d", ind->mtu);
        break;
    }
    case BLE_GAP_DISCONNECTED_IND:
    {
        ble_gap_disconnected_ind_t *ind = (ble_gap_disconnected_ind_t *)data;
        LOG_I("BLE_GAP_DISCONNECTED_IND(%d)", ind->reason);
        break;
    }
    default:
        break;
    }
    return 0;
}
BLE_EVENT_REGISTER(ble_app_event_handler, NULL);

#endif /* BLUETOOTH */

int main(void)
{
    //*(volatile uint32_t *)0x4004f000 = 1;
    MODIFY_REG(hwp_pmuc->LXT_CR, PMUC_LXT_CR_BM_Msk, MAKE_REG_VAL(0xF, PMUC_LXT_CR_BM_Msk, PMUC_LXT_CR_BM_Pos));   // Increase current

#ifdef BSP_USING_PM
    rt_pm_request(PM_SLEEP_MODE_IDLE);
    rt_kprintf("hcpu main!!!\n");
#endif /* BSP_USING_PM */

#ifdef BLUETOOTH
    uint32_t value;
    int ret;
    app_env_t *env = ble_app_get_env();

    env->mb_handle = rt_mb_create("app", 8, RT_IPC_FLAG_FIFO);
    sifli_ble_enable();
    rt_mb_recv(env->mb_handle, (rt_uint32_t *)&value, RT_WAITING_FOREVER);
    if (value == BLE_POWER_ON_IND)
    {
        env->is_power_on = 1;
        env->conn_para.mtu = 23; /* Default value. */
        ble_app_service_init();
        /* First enable connectable adv then enable non-connectable. */
        ble_app_advertising_start();
        LOG_I("receive BLE power on!\r\n");
    }
#endif /* BLUETOOTH */

#ifdef BSP_USING_PM
    rt_thread_delay(5000);
    rt_pm_release(PM_SLEEP_MODE_IDLE);
    app_wakeup();

    if (PM_HIBERNATE_BOOT == SystemPowerOnModeGet())
        rt_kprintf("boot from hibernate!!!\n");
#endif /* BSP_USING_PM */

    while (1)
    {
        rt_thread_mdelay(5000);
        rt_kprintf("hcpu timer wakeup!!!\n");
    }
    return RT_EOK;
}

