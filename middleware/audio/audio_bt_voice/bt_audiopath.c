/*
 * SPDX-FileCopyrightText: 2020-2021 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtconfig.h>
#include "audio_mem.h"

#if (defined(AUDIO_PATH_USING_HCPU) || (defined(AUDIO_PATH_USING_LCPU)))
#include "bf0_mbox_common.h"
#include "log.h"

#ifdef USING_IPC_QUEUE
    #ifdef SOC_BF0_HCPU
        #define SYS_HL_BT_AUDIO_QUEUE    (6)
        static ipc_queue_handle_t sys_hl_bt_audio_queue = IPC_QUEUE_INVALID_HANDLE;
    #endif

    #ifdef SOC_BF0_LCPU
        #define SYS_LH_BT_AUDIO_QUEUE    (6)
        __ROM_USED ipc_queue_handle_t sys_lh_bt_audio_queue;
    #endif
#endif

#define CVSD_MODE               2
#define TRANS_MODE              3


#if defined(SOC_BF0_LCPU)
//#include "audio_cvsd_plc.h"

#ifdef AUDIO_PATH_USING_HCPU
#include "ipc/ringbuffer.h"
#include <rtdevice.h>
#include <rtthread.h>
#include <string.h>
#include <stdlib.h>


#define HCPU_LCPU_SHARE_MEM_BASE_ADDR  LCPU_AUDIO_MEM_START_ADDR


__ROM_USED struct rt_ringbuffer *pt_tx_rbf;
__ROM_USED struct rt_ringbuffer *pt_rx_rbf;
__ROM_USED uint8_t *p_u8_tx_rbf_pool;
__ROM_USED uint8_t *p_u8_rx_rbf_pool;
__ROM_USED struct hci_sync_con_cmp_evt *p_hl_sco_para;
__ROM_USED uint16_t *p_sco_sta;
__ROM_USED uint16_t g_ringbuffer_len;

#define AUDIO_RINGBUFFER_LEN       ((LCPU_HCPU_AUDIO_MEM_SIZE - 0x50 - sizeof(struct hci_sync_con_cmp_evt))/8*4)  //cache data and synchionization

static uint8_t g_bt2speaker_start = 0;
static uint8_t g_mic2bt_start = 0;
static uint32_t  g_total_audio_packet = 0;
static uint32_t  g_error_packetnum = 0;

struct hci_sync_con_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
    ///BD address
    uint8_t  addr[6];;
    ///Link type
    uint8_t lk_type;
    ///Transmit interval
    uint8_t tx_int;
    ///Retransmission window
    uint8_t ret_win;
    ///rx packet length
    uint16_t rx_pkt_len;
    ///tx packet length
    uint16_t tx_pkt_len;
    ///Air mode
    uint8_t air_mode;

};

extern struct hci_sync_con_cmp_evt lc_get_sco_para();

#endif

#define AUDIO_TX_PACKET_MAX        2
static uint8_t g_tx_packet_num = 0;

enum sco_data_stat
{
    SEND_DATA_HANDLE_ERR,
    SEND_DATA_BUFF_ERR,
    SEND_RINGBUFF_EMPTY,
    RECV_RINGBUFF_FULL,
    SCO_PARA_CHG,
    SCO_RX_CNT,
    SCO_STAT_MAX,
};


/// LC BT SCO DATA interface
struct lc_sco_data_tag
{
    /// length of the data
    uint8_t  length;
    uint8_t  packet_status;
    /// reserved for feature use
    uint16_t rsvd1;
    /// data pointer
    uint8_t  *data_ptr;
};
typedef struct bt_sco_callback_para
{
    uint8_t handle_type;
    union
    {
        uint32_t u32_value;
        struct lc_sco_data_tag sco_data;
    } un_para;
} bt_sco_callback_para_t;
enum sco_handle_type
{
#if (!defined(SOC_SF32LB52X)) && (!defined(SF32LB52X_58)) && ((!defined(SF32LB58X)) || (defined(SF32LB58X) && !defined(LCPU_CONFIG_V2)))
    AUDIO_PATH_SCO_INIT,
    AUDIO_PATH_SCO_OPEN,
    AUDIO_PATH_SCO_CONFIG,
    AUDIO_PATH_SCO_TX_START,
    AUDIO_PATH_SCO_RX_START,
    AUDIO_PATH_SCO_TX_STOP,
    AUDIO_PATH_SCO_RX_STOP,
#endif
    AUDIO_PATH_SCO_TX_DATA,
    AUDIO_PATH_SCO_RX_DATA,
    AUDIO_PATH_SCO_CMP_EVT,
    AUDIO_PATH_SCO_CHG_EVT,
    AUDIO_PATH_SCO_DIS_EVT,
    AUDIO_PATH_ALL_SCO_DIS_EVT,
};
//extern struct rt_semaphore g_sco_para_lock; error
//extern uint16_t lc_get_sco_tx_len();
typedef uint8_t (*bt_sco_data_handle_t)(void *p_param);
extern void bt_sco_data_handle_register(bt_sco_data_handle_t fun);

extern uint8_t bt_sco_data_handle_callback(void *p_param);
//extern bt_sco_data_handle_t bt_sco_data_handle_fun;
extern uint8_t lc_bt_sco_data_recv(struct lc_sco_data_tag *p_sco_para);

uint8_t bt_sco_data_handle_callback_lite(void *p_param)
{
    return 0;
}
#if (!defined(SF32LB52X_58)) && ((!defined(SF32LB58X)) || (defined(SF32LB58X) && !defined(LCPU_CONFIG_V2)))
int bt_audiopath_init(void)
{
    pt_tx_rbf = (struct rt_ringbuffer *)HCPU_LCPU_SHARE_MEM_BASE_ADDR;
    pt_rx_rbf = (struct rt_ringbuffer *)(HCPU_LCPU_SHARE_MEM_BASE_ADDR + 0x20);

    p_sco_sta = (uint16_t *)(HCPU_LCPU_SHARE_MEM_BASE_ADDR + 0x40);
    p_u8_tx_rbf_pool = (uint8_t *)(HCPU_LCPU_SHARE_MEM_BASE_ADDR + 0x50); //1KB
    p_u8_rx_rbf_pool = (uint8_t *)(HCPU_LCPU_SHARE_MEM_BASE_ADDR + 0x50 + AUDIO_RINGBUFFER_LEN); //1KB
    p_hl_sco_para = (struct hci_sync_con_cmp_evt *)(HCPU_LCPU_SHARE_MEM_BASE_ADDR + LCPU_HCPU_AUDIO_MEM_SIZE - sizeof(struct hci_sync_con_cmp_evt));


#if 1
    bt_sco_data_handle_register(bt_sco_data_handle_callback);

    rt_ringbuffer_init(pt_tx_rbf, p_u8_tx_rbf_pool, AUDIO_RINGBUFFER_LEN);
    rt_ringbuffer_init(pt_rx_rbf, p_u8_rx_rbf_pool, AUDIO_RINGBUFFER_LEN);

#else
    bt_sco_data_handle_register(bt_sco_data_handle_callback_lite);
#endif
    return 0;
}


__ROM_USED void bt_soc_send_data(uint8_t len)
{
    struct lc_sco_data_tag sco_data;
    rt_size_t bufsize, get_size;
    uint8_t ret;

    sco_data.length = len;
    sco_data.data_ptr = audio_mem_calloc(1, sco_data.length);

    bufsize = rt_ringbuffer_data_len(pt_rx_rbf);
    if (bufsize >= sco_data.length)
    {
        get_size = rt_ringbuffer_get(pt_rx_rbf, sco_data.data_ptr, sco_data.length);
        ret = lc_bt_sco_data_recv(&sco_data);
        if (ret == 1)
        {
            (*(p_sco_sta + SEND_DATA_BUFF_ERR))++;
        }
        else if (ret == 2)
        {
            (*(p_sco_sta + SEND_DATA_HANDLE_ERR))++;
        }
    }
    else
    {
        //send null
        (*(p_sco_sta + SEND_RINGBUFF_EMPTY))++;
    }


    audio_mem_free(sco_data.data_ptr);
}
__ROM_USED uint8_t bt_sco_data_handle_callback(void *p_param)
{
    uint8_t ret = RT_EOK;
    bt_sco_callback_para_t *p_sco_para = (bt_sco_callback_para_t *)p_param;
    struct lc_sco_data_tag *p_sco_data = &(p_sco_para->un_para.sco_data);

    //rt_kprintf("sco_callback:%d\n", p_sco_para->handle_type);

    switch (p_sco_para->handle_type)
    {
    case AUDIO_PATH_SCO_TX_DATA:
    {
        rt_size_t bufsize = 0;
        rt_size_t putnum = 0;

#ifndef SOC_SF32LB52X
        struct hci_sync_con_cmp_evt sco_para_tmp;
        int32_t cmpret;
        sco_para_tmp = lc_get_sco_para();
        cmpret = rt_memcmp(p_hl_sco_para, &sco_para_tmp, sizeof(sco_para_tmp));
        if (cmpret != 0)
        {
            rt_kprintf("old sco coded:%d,rxlen:%d,txlen:%d\n", p_hl_sco_para->air_mode, p_hl_sco_para->rx_pkt_len, p_hl_sco_para->tx_pkt_len);
            *p_hl_sco_para = lc_get_sco_para();
            rt_ringbuffer_reset(pt_tx_rbf);
            rt_ringbuffer_reset(pt_rx_rbf);
            for (int i = 0; i < SCO_STAT_MAX; i++)
            {
                *(p_sco_sta + i) = 0;
            }
            rt_kprintf("new sco coded:%d,rxlen:%d,txlen:%d\n", p_hl_sco_para->air_mode, p_hl_sco_para->rx_pkt_len, p_hl_sco_para->tx_pkt_len);
        }
#endif

        bufsize = rt_ringbuffer_data_len(pt_tx_rbf);

        if (bufsize + p_sco_data->length + 4 <= AUDIO_RINGBUFFER_LEN)
        {

            putnum = rt_ringbuffer_put(pt_tx_rbf, (uint8_t *)p_sco_data, 4);

            putnum = rt_ringbuffer_put(pt_tx_rbf, p_sco_data->data_ptr, p_sco_data->length);

            bt_soc_send_data(p_sco_data->length);

        }
        else
        {
            //drop the receive data
            rt_kprintf("bt2speaker buffer almost full %d\n", bufsize);
            bt_soc_send_data(p_sco_data->length);
            (*(p_sco_sta + RECV_RINGBUFF_FULL))++;
        }
        (*(p_sco_sta + SCO_RX_CNT))++;
        ipc_queue_write(sys_lh_bt_audio_queue, NULL, 0, 0);//every packet notify
#if 0 //loopback test
        rt_ringbuffer_put(pt_rx_rbf, p_sco_data->data_ptr, p_sco_data->length);
#endif
    }
    break;
    case AUDIO_PATH_SCO_RX_DATA:
    {


    }
    break;
    case AUDIO_PATH_SCO_CMP_EVT:
    {
        *p_hl_sco_para = lc_get_sco_para();

        rt_kprintf("sco coded:%d,rxlen:%d,txlen:%d\n", p_hl_sco_para->air_mode, p_hl_sco_para->rx_pkt_len, p_hl_sco_para->tx_pkt_len);

        rt_ringbuffer_reset(pt_tx_rbf);
        rt_ringbuffer_reset(pt_rx_rbf);
        for (int i = 0; i < SCO_STAT_MAX; i++)
        {
            *(p_sco_sta + i) = 0;
        }
    }
    break;
    case AUDIO_PATH_SCO_CHG_EVT:
    {
        (*(p_sco_sta + SCO_PARA_CHG))++;
    }
    break;
    default:
        break;
    }

    return ret;;
}


INIT_APP_EXPORT(bt_audiopath_init);
#else
#include "ble_stack.h"
ble_mem_config_t *ble_memory_config_get(void);
uint8_t rom_config_get_bt_sco_max_act(void);
void bt_sco_data_handle_register(bt_sco_data_handle_t fun);
void regist_sco_para_save_ptr(struct hci_sync_con_cmp_evt *ptr);
uint8_t rom_config_get_default_sco_has_txhdr(void);
void *lc_get_sco_para_ptr_for3sco(uint8_t sco_linkid);
uint8_t *rom_config_get_audio_buffer(void);
uint16_t rom_config_get_aud_buf_size(void);
uint8_t *g_sco_buf = NULL;
#define MAX_SCO_DATA_LEN 250

RT_WEAK uint8_t *rom_config_get_audio_buffer(void)
{
    return NULL;
}
RT_WEAK uint16_t rom_config_get_aud_buf_size(void)
{
    return 0;
}
RT_WEAK void *lc_get_sco_para_ptr_for3sco(uint8_t sco_linkid)
{
    return NULL;
}
RT_WEAK uint8_t rom_config_get_default_sco_has_txhdr(void)
{
    return 0;
}
RT_WEAK void regist_sco_para_save_ptr(struct hci_sync_con_cmp_evt *ptr)
{
}
RT_WEAK uint8_t rom_config_get_bt_sco_max_act(void)
{
    return 1;
}
void bt_soc_send_data(uint8_t len);
typedef int (*bt_audiopath_init_handler)(void);
typedef void (*bt_soc_send_data_handler)(uint8_t len);
typedef uint8_t (*bt_sco_data_handle_callback_handler)(void *p_param);
__USED bt_audiopath_init_handler bt_audiopath_init_start = NULL;
__USED bt_audiopath_init_handler bt_audiopath_init_end = NULL;
__USED bt_soc_send_data_handler bt_soc_send_data_fun = bt_soc_send_data;
__USED bt_soc_send_data_handler bt_soc_send_data_end = NULL;
__USED bt_sco_data_handle_callback_handler bt_sco_data_handle_callback_fun = bt_sco_data_handle_callback;
__USED bt_sco_data_handle_callback_handler bt_sco_data_handle_callback_end = NULL;

__ROM_USED int bt_audiopath_init(void)
{
    uint8_t *audio_addr = (uint8_t *)HCPU_LCPU_SHARE_MEM_BASE_ADDR;
    uint16_t audio_len = LCPU_HCPU_AUDIO_MEM_SIZE;
    uint16_t ringbuffer_len;
    uint8_t sco_num = rom_config_get_bt_sco_max_act();

    if (bt_audiopath_init_start)
    {
        bt_audiopath_init_start();
    }

    if (rom_config_get_audio_buffer())
    {
        audio_addr = rom_config_get_audio_buffer();
        audio_len = rom_config_get_aud_buf_size();
    }
    ringbuffer_len = ((audio_len - 0x50 - sco_num * sizeof(struct hci_sync_con_cmp_evt)) / 8 * 4);
    g_ringbuffer_len = ringbuffer_len;

    pt_tx_rbf = (struct rt_ringbuffer *)audio_addr;
    pt_rx_rbf = (struct rt_ringbuffer *)(audio_addr + 0x20);

    p_sco_sta = (uint16_t *)(audio_addr + 0x40);
    p_u8_tx_rbf_pool = (uint8_t *)(audio_addr + 0x50); //1KB
    p_u8_rx_rbf_pool = (uint8_t *)(audio_addr + 0x50 + ringbuffer_len); //1KB
    p_hl_sco_para = (struct hci_sync_con_cmp_evt *)(audio_addr + audio_len - sizeof(struct hci_sync_con_cmp_evt) * sco_num);

    bt_sco_data_handle_register(bt_sco_data_handle_callback_fun);
    regist_sco_para_save_ptr(p_hl_sco_para);

    rt_ringbuffer_init(pt_tx_rbf, p_u8_tx_rbf_pool, ringbuffer_len);
    rt_ringbuffer_init(pt_rx_rbf, p_u8_rx_rbf_pool, ringbuffer_len);

    g_sco_buf = audio_mem_calloc(1, MAX_SCO_DATA_LEN);
    rt_ringbuffer_reset(pt_tx_rbf);
    rt_ringbuffer_reset(pt_rx_rbf);
    for (int i = 0; i < SCO_STAT_MAX; i++)
    {
        *(p_sco_sta + i) = 0;
    }

    if (bt_audiopath_init_end)
    {
        bt_audiopath_init_end();
    }

    return 0;
}


__ROM_USED void bt_soc_send_data(uint8_t len)
{
    struct lc_sco_data_tag sco_data;
    rt_size_t bufsize, get_size;
    uint8_t ret;

    if (rom_config_get_default_sco_has_txhdr())
    {
        sco_data.length = len + 4;
    }
    else
    {
        sco_data.length = len;
    }

    RT_ASSERT(sco_data.length < MAX_SCO_DATA_LEN);
    sco_data.data_ptr = g_sco_buf;

    bufsize = rt_ringbuffer_data_len(pt_rx_rbf);
    if (bufsize >= sco_data.length)
    {
        get_size = rt_ringbuffer_get(pt_rx_rbf, sco_data.data_ptr, sco_data.length);
        ret = lc_bt_sco_data_recv(&sco_data);
        if (ret == 1)
        {
            (*(p_sco_sta + SEND_DATA_BUFF_ERR))++;
        }
        else if (ret == 2)
        {
            (*(p_sco_sta + SEND_DATA_HANDLE_ERR))++;
        }
    }
    else
    {
        //send null
        (*(p_sco_sta + SEND_RINGBUFF_EMPTY))++;
    }

    if (bt_soc_send_data_end)
    {
        bt_soc_send_data_end(len);
    }
    //audio_mem_free(sco_data.data_ptr);
}
__ROM_USED uint8_t bt_sco_data_handle_callback(void *p_param)
{
    uint8_t ret = RT_EOK;
    bt_sco_callback_para_t *p_sco_para = (bt_sco_callback_para_t *)p_param;
    struct lc_sco_data_tag *p_sco_data = &(p_sco_para->un_para.sco_data);

    switch (p_sco_para->handle_type)
    {
    case AUDIO_PATH_SCO_TX_DATA:
    {
        rt_size_t bufsize = 0;
        rt_size_t putnum = 0;

#if (!defined(SOC_SF32LB52X)) && (!defined(SF32LB52X_58)) && ((!defined(SF32LB58X)) || (defined(SF32LB58X) && !defined(LCPU_CONFIG_V2)))
        struct hci_sync_con_cmp_evt sco_para_tmp;
        int32_t cmpret;
        sco_para_tmp = lc_get_sco_para();
        cmpret = rt_memcmp(p_hl_sco_para, &sco_para_tmp, sizeof(sco_para_tmp));
        if (cmpret != 0)
        {
            rt_kprintf("old sco coded:%d,rxlen:%d,txlen:%d\n", p_hl_sco_para->air_mode, p_hl_sco_para->rx_pkt_len, p_hl_sco_para->tx_pkt_len);
            *p_hl_sco_para = lc_get_sco_para();
            rt_ringbuffer_reset(pt_tx_rbf);
            rt_ringbuffer_reset(pt_rx_rbf);
            for (int i = 0; i < SCO_STAT_MAX; i++)
            {
                *(p_sco_sta + i) = 0;
            }
            rt_kprintf("new sco coded:%d,rxlen:%d,txlen:%d\n", p_hl_sco_para->air_mode, p_hl_sco_para->rx_pkt_len, p_hl_sco_para->tx_pkt_len);
        }
#endif

        bufsize = rt_ringbuffer_data_len(pt_tx_rbf);

        if (bufsize + p_sco_data->length + 4 <= g_ringbuffer_len)
        {
            //memcpy((p_sco_data->data_ptr - 4), p_sco_data, 4);
            //putnum = rt_ringbuffer_put(pt_tx_rbf, (p_sco_data->data_ptr - 4), (p_sco_data->length + 4));
            memcpy(g_sco_buf, p_sco_data, 4);
            memcpy(g_sco_buf + 4, p_sco_data->data_ptr, p_sco_data->length);
            putnum = rt_ringbuffer_put(pt_tx_rbf, (g_sco_buf), (p_sco_data->length + 4));

            bt_soc_send_data_fun(p_sco_data->length);

        }
        else
        {
            //drop the receive data
            //rt_kprintf("bt2speaker buffer almost full %d\n", bufsize);
            bt_soc_send_data_fun(p_sco_data->length);
            (*(p_sco_sta + RECV_RINGBUFF_FULL))++;
        }
        if ((*(p_sco_sta + SCO_RX_CNT) & 0x3FF) == 0)
        {
            uint32_t *p32_sco_sta = (uint32_t *)p_sco_sta;
            rt_kprintf("lcpu sco, buferrcnt:0x%x, buf full_empty:0x%x, rx cnt:0x%x\n", *p32_sco_sta, *(p32_sco_sta + 1), *(p32_sco_sta + 2));
        }
        (*(p_sco_sta + SCO_RX_CNT))++;
        ipc_queue_write(sys_lh_bt_audio_queue, NULL, 0, 0);//every packet notify
#if 0 //loopback test
        rt_ringbuffer_put(pt_rx_rbf, p_sco_data->data_ptr, p_sco_data->length);
#endif
    }
    break;
    case AUDIO_PATH_SCO_RX_DATA:
    {


    }
    break;
    case AUDIO_PATH_SCO_CMP_EVT:
    {
        uint8_t sco_linkid = p_sco_para->un_para.u32_value & 0xFF;
        struct hci_sync_con_cmp_evt *pt_sco_para = lc_get_sco_para_ptr_for3sco(sco_linkid);
        uint16_t scolinkhdl = (p_sco_para->un_para.u32_value >> 16) & 0xFFFF;
        //*pt_sco_para = lc_get_sco_para(sco_linkid);

        rt_kprintf("sco linkid:0x%x,coded:%d,rxlen:%d,txlen:%d\n", scolinkhdl, pt_sco_para->air_mode, pt_sco_para->rx_pkt_len, pt_sco_para->tx_pkt_len);
#if !defined(SF32LB52X_58) && ((!defined(SF32LB58X)) || (defined(SF32LB58X) && !defined(LCPU_CONFIG_V2)))
        rt_ringbuffer_reset(pt_tx_rbf);
        rt_ringbuffer_reset(pt_rx_rbf);
        for (int i = 0; i < SCO_STAT_MAX; i++)
        {
            *(p_sco_sta + i) = 0;
        }
#endif
    }
    break;
    case AUDIO_PATH_SCO_CHG_EVT:
    {
        uint8_t sco_linkid = p_sco_para->un_para.u32_value & 0xFF;
        struct hci_sync_con_cmp_evt *pt_sco_para = lc_get_sco_para_ptr_for3sco(sco_linkid);
        uint16_t scolinkhdl = (p_sco_para->un_para.u32_value >> 16) & 0xFFFF;

        rt_kprintf("sco change linkid:0x%x,coded:%d,rxlen:%d,txlen:%d\n", scolinkhdl, pt_sco_para->air_mode, pt_sco_para->rx_pkt_len, pt_sco_para->tx_pkt_len);

        (*(p_sco_sta + SCO_PARA_CHG))++;
    }
    break;
    case AUDIO_PATH_SCO_DIS_EVT:
    {
        //uint8_t sco_linkid = p_sco_para->un_para.u32_value & 0xFF;
        //struct hci_sync_con_cmp_evt *pt_sco_para = p_hl_sco_para + sco_linkid;
        uint16_t scolinkhdl = (p_sco_para->un_para.u32_value >> 16) & 0xFFFF;
        //*pt_sco_para = lc_get_sco_para();

        rt_kprintf("sco linkid:0x%x disconnected\n", scolinkhdl);

        break;
    }
    case AUDIO_PATH_ALL_SCO_DIS_EVT:
    {
        rt_ringbuffer_reset(pt_tx_rbf);
        rt_ringbuffer_reset(pt_rx_rbf);
        for (int i = 0; i < SCO_STAT_MAX; i++)
        {
            *(p_sco_sta + i) = 0;
        }
    }
    break;
    default:
        break;
    }

    if (bt_sco_data_handle_callback_end)
    {
        bt_sco_data_handle_callback_end(p_param);
    }

    return ret;;
}


INIT_APP_EXPORT(bt_audiopath_init);

#endif


#ifdef AUDIO_PATH_USING_HCPU

__ROM_USED int32_t lh_bt_audio_queue_rx_ind(ipc_queue_handle_t handle, size_t size)
{
    //LOG_E("lh_bt_audio_queue_rx_ind");
    g_mic2bt_start = 0;
    g_bt2speaker_start = 0;
    g_tx_packet_num = 0;
    //rt_kprintf("total_packer:%d,err_packer:%d\n", g_total_audio_packet, g_error_packetnum);
    g_total_audio_packet = 0;
    g_error_packetnum = 0;
    return 0;
}
__ROM_USED int sys_init_lh_bt_audio_queue(void)
{
    ipc_queue_cfg_t q_cfg;

    q_cfg.qid = SYS_LH_BT_AUDIO_QUEUE;
    q_cfg.tx_buf_size = 0;
    q_cfg.tx_buf_addr = (uint32_t)NULL;
    q_cfg.tx_buf_addr_alias = (uint32_t)NULL;
    q_cfg.rx_buf_addr = (uint32_t)NULL;
    q_cfg.rx_ind = lh_bt_audio_queue_rx_ind;
    q_cfg.user_data = 0;

    sys_lh_bt_audio_queue = ipc_queue_init(&q_cfg);

    RT_ASSERT(IPC_QUEUE_INVALID_HANDLE != sys_lh_bt_audio_queue);

    ipc_queue_open(sys_lh_bt_audio_queue);

    return 0;
}
INIT_APP_EXPORT(sys_init_lh_bt_audio_queue);
#endif

#endif


#endif

