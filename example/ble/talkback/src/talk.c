#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <stdlib.h>
#include "os_adaptor.h"
#include "mem_section.h"

#include "main.h"

#if defined(PKG_LIB_OPUS) && defined(RT_USING_FINSH) && defined(AUDIO_USING_MANAGER)
#include "opus_types.h"
#include "opus_private.h"
#include "opus_multistream.h"

#include "os_support.h"
#include "audio_server.h"

#undef LOG_TAG

#define LOG_TAG           "opus"
#define DBG_TAG           "opus"
#define DBG_LVL           LOG_LVL_INFO
#include "log.h"

//#define SENDER_DEBUG_MIC2SPEAKER    1  //voice sender debug, loop mic to local speaker before sending to peer

#define MAX_MIXER_NUM               3

#define OPUS_USING_20MS             1
#define CODEC_DATA_UNIT_LEN         320
#define OPUS_STACK_SIZE             32000

#define OPUS_PACKET_HEADER          0x88
#define TALK_THREAD_NAME            "opusble"
#define OPUS_THREAD_PRIORITY        (RT_THREAD_PRIORITY_MIDDLE + RT_THREAD_PRIORITY_HIGHER)
#define OPUS_EVENT_MIC_RX           (1 << 0)
#define OPUS_EVENT_SPK_TX           (1 << 1)
#define OPUS_EVENT_BLE_DOWNLINK     (1 << 2)
#define OPUS_ENCODE_QUEUE_NUM       3
#define OPUS_SAMPLERATE             8000 //16000

#if OPUS_SAMPLERATE == 16000
    #define OPUS_DOWNLINK_QUEUE_NUM     64
#else
    #define OPUS_DOWNLINK_QUEUE_NUM     32
#endif

#define WRITE_CACHE_SIZE                (CODEC_DATA_UNIT_LEN * OPUS_DOWNLINK_QUEUE_NUM / 2)

#if OPUS_USING_20MS
    #if OPUS_SAMPLERATE == 16000
        #define OPUS_ENCODE_DATA_LEN    (CODEC_DATA_UNIT_LEN * 2)
    #else
        #define OPUS_ENCODE_DATA_LEN    (CODEC_DATA_UNIT_LEN)
    #endif
#else
    #if OPUS_SAMPLERATE == 16000
        #define OPUS_ENCODE_DATA_LEN    (CODEC_DATA_UNIT_LEN)
    #else
        #define OPUS_ENCODE_DATA_LEN    (CODEC_DATA_UNIT_LEN/2)
    #endif
#endif

#define OPUS_EVENT_ALL  (OPUS_EVENT_MIC_RX | OPUS_EVENT_SPK_TX | OPUS_EVENT_BLE_DOWNLINK)

#pragma pack(push, 1)
typedef struct
{
    rt_slist_t  slist;
    uint8_t     id;
    uint8_t     len;
    uint8_t     data[OPUS_ENCODE_DATA_LEN];
} ble_opus_packet_t;

typedef struct
{
    rt_slist_t  node;
    uint8_t     *data;
    uint16_t    data_len;
    uint16_t    size;
} opus_decode_queue_t;

typedef enum
{
    OPUS_WAIT_ID,
    OPUS_WAIT_LEN,
    OPUS_WAIT_DATA,
} opus_packet_state_t;

#pragma pack(pop)

typedef struct
{
    OpusDecoder             *decoder;
    opus_decode_queue_t     downlink_queue[OPUS_DOWNLINK_QUEUE_NUM];
    uint8_t                 downlink_data[OPUS_ENCODE_DATA_LEN];
    uint16_t                downlink_data_len;
    uint16_t                downlink_data_pos;
    uint8_t                 downlink_decode_out[OPUS_ENCODE_DATA_LEN];
    rt_slist_t              downlink_decode_busy;
    rt_slist_t              downlink_decode_idle;
    uint8_t                 recv_id;
    uint8_t                 readed_id;
    uint8_t                 is_got_first_id;
    uint8_t                 actv_idx;
    uint8_t                 is_used;
    opus_packet_state_t     state;
} ble_mixer_data_t;

typedef struct
{
    ble_mixer_data_t        downlink_mixers[MAX_MIXER_NUM];
    rt_slist_t              encode_slist;
    ble_opus_packet_t       encode_one[OPUS_ENCODE_QUEUE_NUM];
    uint8_t                 send_data[OPUS_ENCODE_DATA_LEN * OPUS_ENCODE_QUEUE_NUM + 10 * OPUS_ENCODE_QUEUE_NUM];

    struct rt_ringbuffer    *rbuf_out;
    os_thread_t             opus_thread;
    audio_client_t          client;
    OpusEncoder             *encoder;
    struct rt_ringbuffer    *rb_opus_encode_input;
    rt_event_t              event;
    audio_rwflag_t          flag;
    uint16_t                speaker_cached_packets;
    uint8_t                 is_to_speaker_enable;
    uint8_t                 is_first_encoded;
    uint8_t                 mic_rx_count;
    uint8_t                 send_id;
    uint8_t                 is_tx_enable;
    uint8_t                 is_rx_enable;
    uint8_t                 is_exit;
    uint8_t                 is_inited;
} ble_talker_t;

static ble_talker_t *g_talker = NULL;

L2_RET_BSS_SECT_BEGIN(g_opus_stack)
static  uint32_t g_opus_stack[OPUS_STACK_SIZE / sizeof(uint32_t)] L2_RET_BSS_SECT(g_opus_stack);
L2_RET_BSS_SECT_END

/*
    call in audio server thread, see audio_server.c
*/
static int audio_callback(audio_server_callback_cmt_t cmd, void *callback_userdata, uint32_t reserved)
{
    ble_talker_t *thiz = (ble_talker_t *)callback_userdata;

    if (thiz->is_tx_enable && (cmd == as_callback_cmd_data_coming))
    {
        audio_server_coming_data_t *p = (audio_server_coming_data_t *)reserved;
        rt_ringbuffer_put(thiz->rb_opus_encode_input, p->data, p->data_len);
        thiz->mic_rx_count++;
#if OPUS_SAMPLERATE == 16000
        if (thiz->mic_rx_count > 1)
#endif
        {
            thiz->mic_rx_count = 0;
            rt_event_send(thiz->event, OPUS_EVENT_MIC_RX);
        }
    }
    else if (thiz->is_rx_enable && (cmd == as_callback_cmd_cache_half_empty || cmd == as_callback_cmd_cache_empty))
    {
        //rt_event_send(thiz->event, OPUS_EVENT_SPK_TX);
    }
    return 0;
}

static ble_mixer_data_t *get_mixer(ble_talker_t *thiz, uint8_t actv_idx)
{
    ble_mixer_data_t *mixer = NULL;
    rt_enter_critical();

    for (int m = 0; m < MAX_MIXER_NUM; m++)
    {
        if (thiz->downlink_mixers[m].is_used
                && thiz->downlink_mixers[m].actv_idx == actv_idx)
        {
            mixer = &thiz->downlink_mixers[m];
        }
    }

    if (!mixer)
    {
        for (int m = 0; m < MAX_MIXER_NUM; m++)
        {
            if (!thiz->downlink_mixers[m].is_used)
            {
                mixer = &thiz->downlink_mixers[m];
                mixer->is_got_first_id = 0;
                mixer->state = OPUS_WAIT_ID;
                mixer->is_used = 1;
                mixer->actv_idx = actv_idx;
                break;
            }
        }
    }

    rt_exit_critical();

    return mixer;
}

void ble_talk_downlink(uint8_t actv_idx, uint8_t *data, uint16_t data_len)
{
    /*
        todo:
            this ble thread should using critical with opus_thread_entry for access g_talker
    */

    if (!g_talker || !data_len || !g_talker->is_rx_enable || !g_talker->is_inited)
    {
        return;
    }


    ble_mixer_data_t *mixer;
    ble_talker_t *thiz = (ble_talker_t *)g_talker;

    //LOG_HEX("recv:", 16, data,data_len);

    mixer = get_mixer(thiz, actv_idx);
    //LOG_I("actv_idx %d using mixer=0x%p", actv_idx, mixer);

    if (!mixer)
    {
        return;
    }

    while (g_talker && g_talker->is_inited && data_len > 0)
    {
        switch (mixer->state)
        {
        case OPUS_WAIT_ID:
            mixer->recv_id = *data++;
            mixer->state = OPUS_WAIT_LEN;
            data_len--;
            //LOG_I("actv_idx %d id: recv=%d readed=%d", actv_idx, mixer->recv_id, mixer->readed_id);
            break;

        case OPUS_WAIT_LEN:
        {
            mixer->downlink_data_len = *data++;;
            mixer->state = OPUS_WAIT_DATA;
            data_len--;
            if (mixer->downlink_data_len == 0)
            {
                LOG_I("downlink err len=%02X", mixer->downlink_data_len);
                mixer->state = OPUS_WAIT_ID;
            }
            mixer->downlink_data_pos = 0;
            break;
        }
        case OPUS_WAIT_DATA:
        {
            uint32_t remain;
            remain = mixer->downlink_data_len - mixer->downlink_data_pos;
            if (remain > data_len)
            {
                memcpy(mixer->downlink_data + mixer->downlink_data_pos, data, data_len);
                mixer->downlink_data_pos += data_len;
                data_len = 0;
                data += data_len;
                break;
            }
            memcpy(mixer->downlink_data + mixer->downlink_data_pos, data, remain);
            data_len -= remain;
            data += remain;
            mixer->downlink_data_pos = 0;
            mixer->state = OPUS_WAIT_ID;
            int is_old = 0;
            //LOG_HEX("got opus", 16, thiz->downlink_data, thiz->downlink_data_len);
            if (mixer->is_got_first_id)
            {
                if (mixer->recv_id == (uint8_t)(mixer->readed_id + 1))
                {
                    //right packet
                    mixer->readed_id = mixer->recv_id;
                    //LOG_I("actv_idx %d new=%d", actv_idx, mixer->readed_id);
                }
                else
                {
                    int is_lost = 1;
                    for (int i = 0; i < OPUS_ENCODE_QUEUE_NUM; i++)
                    {
                        if ((uint8_t)(mixer->recv_id + i) == (uint8_t)mixer->readed_id)
                        {
                            is_lost = 0; //old packet
                            is_old = 1;
                            break;
                        }
                    }
                    if (is_lost)
                    {
                        //LOG_I("actv_idx %d lost %d packet, recv=%d readed=%d", actv_idx, (uint8_t)(mixer->recv_id - mixer->readed_id), mixer->recv_id, mixer->readed_id);
                        mixer->readed_id = mixer->recv_id;
                    }
                }
            }
            else
            {
                mixer->is_got_first_id = 1;
                mixer->readed_id = mixer->recv_id;
            }
            if (is_old)
            {
                break;
            }
            rt_slist_t *idle;
            rt_enter_critical();
            idle = rt_slist_first(&mixer->downlink_decode_idle);
            rt_exit_critical();
            if (idle)
            {
                opus_decode_queue_t *queue = rt_container_of(idle, opus_decode_queue_t, node);
                if (queue->size < mixer->downlink_data_len)
                {
                    rt_free(queue->data);
                    queue->size = mixer->downlink_data_len;
                    queue->data = rt_malloc(queue->size);
                    RT_ASSERT(queue->data);
                }
                queue->data_len = mixer->downlink_data_len;
                memcpy(queue->data, mixer->downlink_data, queue->data_len);

                rt_enter_critical();
                rt_slist_remove(&mixer->downlink_decode_idle, idle);
                rt_slist_append(&mixer->downlink_decode_busy, idle);
                rt_exit_critical();

                if (thiz->is_to_speaker_enable)
                {
                    rt_event_send(thiz->event, OPUS_EVENT_BLE_DOWNLINK);
                }
                else
                {
                    thiz->speaker_cached_packets++;
                    if (thiz->speaker_cached_packets > OPUS_DOWNLINK_QUEUE_NUM / 4)
                    {
                        thiz->is_to_speaker_enable = 1;
                    }
                }
            }
            else
            {
                //LOG_I("drop: no recv buf, actv_idx %d  recv id %d readed id=%d", actv_idx, mixer->recv_id, mixer->readed_id);
                mixer->readed_id = mixer->recv_id;
                //mixer->readed_id +=3; //drop more 3 packets
            }
            break;
        }
        default:
            RT_ASSERT(0);
        }
    }
}

static void audio_write_and_wait(ble_talker_t *thiz, uint8_t *data, uint32_t data_len)
{
    int ret;
    while (!thiz->is_exit)
    {
        ret = audio_write(thiz->client, data, data_len);
        if (ret)
        {
            break;
        }

        rt_thread_mdelay(10);
    }
}
static inline void mix_2_and_write(ble_talker_t *thiz, opus_int16 *p0, opus_int16 *p1, opus_int16 *mix_out)
{
    for (int i = 0; i < OPUS_ENCODE_DATA_LEN / 2; i++)
    {
        mix_out[i] = (opus_int16)(((int)p0[i] + (int)p1[i]) >> 1);
    }
    audio_write_and_wait(thiz, (uint8_t *)mix_out, OPUS_ENCODE_DATA_LEN);
}
static void opus_thread_entry(void *p)
{
    ble_talker_t *thiz = (ble_talker_t *)p;
    int err;

    rt_kprintf("opus runing stack var address =0x%x\r\n", &thiz);

    for (int m = 0; m < MAX_MIXER_NUM; m++)
    {
        ble_mixer_data_t *mixer = &thiz->downlink_mixers[m];
        //LOG_I("mixer %d=0x%p", m, mixer);
        rt_slist_init(&mixer->downlink_decode_idle);
        rt_slist_init(&mixer->downlink_decode_busy);
        for (int i = 0; i < OPUS_DOWNLINK_QUEUE_NUM; i++)
        {
            mixer->downlink_queue[i].size = 32;
            mixer->downlink_queue[i].data = rt_malloc(mixer->downlink_queue[i].size);
            RT_ASSERT(mixer->downlink_queue[i].data);
            rt_slist_append(&mixer->downlink_decode_idle, &mixer->downlink_queue[i].node);
        }
        mixer->state = OPUS_WAIT_ID;
        mixer->downlink_data_pos = 0;
        mixer->decoder = opus_decoder_create(OPUS_SAMPLERATE, 1, &err);
        RT_ASSERT(mixer->decoder);
    }
    rt_slist_init(&thiz->encode_slist);
    for (int i = 0; i < OPUS_ENCODE_QUEUE_NUM; i++)
    {
        rt_slist_append(&thiz->encode_slist, &thiz->encode_one[i].slist);
    }
    thiz->rb_opus_encode_input  = rt_ringbuffer_create(640 * 3); // three 20ms frame

    bool dbg = thiz->rb_opus_encode_input;

    RT_ASSERT(dbg);

    thiz->event = rt_event_create("opusble", RT_IPC_FLAG_FIFO);
    RT_ASSERT(thiz->event);

    thiz->encoder = opus_encoder_create(OPUS_SAMPLERATE, 1, OPUS_APPLICATION_VOIP, &err);
    RT_ASSERT(thiz->encoder);
#if OPUS_USING_20MS
    opus_encoder_ctl(thiz->encoder, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS));
#else
    opus_encoder_ctl(thiz->encoder, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_10_MS));
#endif

    opus_encoder_ctl(thiz->encoder, OPUS_SET_VBR(1));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_VBR_CONSTRAINT(1));

    opus_encoder_ctl(thiz->encoder, OPUS_SET_BITRATE(OPUS_SAMPLERATE));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_COMPLEXITY(0));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_LSB_DEPTH(16));

    opus_encoder_ctl(thiz->encoder, OPUS_SET_DTX(0));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_INBAND_FEC(0));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_PACKET_LOSS_PERC(0));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_PREDICTION_DISABLED(0));

    opus_encoder_ctl(thiz->encoder, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_BANDWIDTH(OPUS_AUTO));
    opus_encoder_ctl(thiz->encoder, OPUS_SET_FORCE_MODE(MODE_SILK_ONLY));

    audio_parameter_t pa = {0};
    pa.write_bits_per_sample = 16;
    pa.write_channnel_num = 1;
    pa.write_samplerate = OPUS_SAMPLERATE;
    pa.read_bits_per_sample = 16;
    pa.read_channnel_num = 1;
    pa.read_samplerate = OPUS_SAMPLERATE;
    pa.read_cache_size = 0;
    pa.write_cache_size = WRITE_CACHE_SIZE;
    pa.is_need_3a = 1;

    thiz->client = audio_open(AUDIO_TYPE_LOCAL_RECORD, AUDIO_TXRX, &pa, audio_callback, thiz);

    thiz->is_inited = 1;

    while (!thiz->is_exit)
    {
        rt_uint32_t evt = 0;
        rt_event_recv(thiz->event, OPUS_EVENT_ALL, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &evt);
        if (evt & OPUS_EVENT_MIC_RX)
        {
            /*
                every time send three packet, two old and one new, packet sequeue number from 0,
                use three node list, move head to tail, and upate tail's data, then send three packet out
                this time send three packet sequeue number  5, 6, 7
                next time send three packet sequeue number  6, 7, 8
                send1:  0    0     0
                send2:  0    0     1
                send3:  0    1     2
                send4:  1    2     3
                send5:  2    3     4
                   ......
            */

            int16_t buf[OPUS_ENCODE_DATA_LEN / 2];
            rt_ringbuffer_get(thiz->rb_opus_encode_input, (uint8_t *)&buf[0], OPUS_ENCODE_DATA_LEN);

            /*move first to tail, and update data & it's new packet sequeue number*/
            rt_slist_t *first = rt_slist_first(&thiz->encode_slist);
            RT_ASSERT(first);
            rt_slist_remove(&thiz->encode_slist, first);
            rt_slist_append(&thiz->encode_slist, first);
            ble_opus_packet_t *cur = rt_container_of(first, ble_opus_packet_t, slist);
            opus_int32 len = opus_encode(thiz->encoder, (const opus_int16 *)buf, OPUS_ENCODE_DATA_LEN / 2, cur->data, OPUS_ENCODE_DATA_LEN);
            if (len < 0 || len > OPUS_ENCODE_DATA_LEN)
            {
                RT_ASSERT(0);
            }

            cur->id = thiz->send_id++;
            cur->len = (uint8_t)len;
            RT_ASSERT(len < 256);

#if SENDER_DEBUG_MIC2SPEAKER
            audio_write(thiz->client, (uint8_t *)&buf[0], OPUS_ENCODE_DATA_LEN);
#endif
            //LOG_I("id=%d", cur->id);

            if (!thiz->is_first_encoded)
            {
                thiz->is_first_encoded = 1;
                rt_slist_t *head_list;
                ble_opus_packet_t *head;
                /*first time: three packet is same packet 0 0 0 */
                for (int i = 0; i < OPUS_ENCODE_QUEUE_NUM - 1; i++)
                {
                    head_list = rt_slist_first(&thiz->encode_slist);
                    RT_ASSERT(head_list);
                    rt_slist_remove(&thiz->encode_slist, head_list);
                    head = rt_container_of(head_list, ble_opus_packet_t, slist);
                    memcpy(head, cur, sizeof(ble_opus_packet_t));
                    rt_slist_append(&thiz->encode_slist, head_list);
                }
            }
            int d = 0, pos = 0;
            rt_slist_t *iter;

            /*send out three packet*/
            rt_slist_for_each(iter, &thiz->encode_slist)
            {
                ble_opus_packet_t *cur = rt_container_of(iter, ble_opus_packet_t, slist);
                d++;
                thiz->send_data[pos++] = cur->id;
                thiz->send_data[pos++] = cur->len;
                memcpy(&thiz->send_data[pos], cur->data, cur->len);
                pos += cur->len;
            }
            RT_ASSERT(d == OPUS_ENCODE_QUEUE_NUM);
            //LOG_HEX("packet", 16, thiz->send_data, pos);
            app_send_voice_data(pos, thiz->send_data);

            if (rt_ringbuffer_data_len(thiz->rb_opus_encode_input) >= OPUS_ENCODE_DATA_LEN)
            {
                rt_event_send(thiz->event, OPUS_EVENT_MIC_RX);
            }
        }

        if (evt & OPUS_EVENT_SPK_TX)
        {
        }

        if (evt & OPUS_EVENT_BLE_DOWNLINK)
        {
            rt_slist_t *decode[MAX_MIXER_NUM + 16] = {0};
            ble_mixer_data_t *mixers[MAX_MIXER_NUM];
            for (int i = 0; i < MAX_MIXER_NUM; i++)
            {
                decode[i] = NULL;
                mixers[i] = &thiz->downlink_mixers[i];
            }

            rt_enter_critical();
            for (int i = 0; i < MAX_MIXER_NUM; i++)
            {
                decode[i] = rt_slist_first(&mixers[i]->downlink_decode_busy);
                if (!decode[i])
                {
                    mixers[i]->is_used = 0;
                }
            }
            rt_exit_critical();

            for (int i = 0; i < MAX_MIXER_NUM; i++)
            {
                if (!decode[i])
                {
                    continue;
                }
                opus_decode_queue_t *queue = rt_container_of(decode[i], opus_decode_queue_t, node);
                opus_int32 res = opus_decode(mixers[i]->decoder,
                                             (const uint8_t *)queue->data,
                                             queue->data_len,
                                             (opus_int16 *)&mixers[i]->downlink_decode_out[0],
                                             OPUS_ENCODE_DATA_LEN, 0);

                if (res != OPUS_ENCODE_DATA_LEN / 2)
                {
                    LOG_I("decode out samples=%d\n", res);
                    RT_ASSERT(0);
                }
            }

            if (decode[0] && decode[1] && decode[2])
            {
                opus_int16 mix_out[OPUS_ENCODE_DATA_LEN / 2];
                opus_int16 *p0 = (opus_int16 *)mixers[0]->downlink_decode_out;
                opus_int16 *p1 = (opus_int16 *)mixers[1]->downlink_decode_out;
                opus_int16 *p2 = (opus_int16 *)mixers[2]->downlink_decode_out;
                for (int i = 0; i < OPUS_ENCODE_DATA_LEN / 2; i++)
                {
                    mix_out[i] = (opus_int16)(((int)p0[i] + (int)p1[i] + (int)p2[i]) / 3);
                }
                audio_write_and_wait(thiz, (uint8_t *)mix_out, OPUS_ENCODE_DATA_LEN);
            }
            else if (decode[0] && decode[1])
            {
                opus_int16 mix_out[OPUS_ENCODE_DATA_LEN / 2];
                opus_int16 *p0 = (opus_int16 *)mixers[0]->downlink_decode_out;
                opus_int16 *p1 = (opus_int16 *)mixers[1]->downlink_decode_out;
                mix_2_and_write(thiz, p0, p1, mix_out);
            }
            else if (decode[0] && decode[2])
            {
                opus_int16 mix_out[OPUS_ENCODE_DATA_LEN / 2];
                opus_int16 *p0 = (opus_int16 *)mixers[0]->downlink_decode_out;
                opus_int16 *p1 = (opus_int16 *)mixers[2]->downlink_decode_out;
                mix_2_and_write(thiz, p0, p1, mix_out);
            }
            else if (decode[1] && decode[2])
            {
                opus_int16 mix_out[OPUS_ENCODE_DATA_LEN / 2];
                opus_int16 *p0 = (opus_int16 *)mixers[1]->downlink_decode_out;
                opus_int16 *p1 = (opus_int16 *)mixers[2]->downlink_decode_out;
                mix_2_and_write(thiz, p0, p1, mix_out);
            }
            else if (decode[0])
            {
                audio_write_and_wait(thiz, mixers[0]->downlink_decode_out, OPUS_ENCODE_DATA_LEN);
            }
            else if (decode[1])
            {
                audio_write_and_wait(thiz, mixers[1]->downlink_decode_out, OPUS_ENCODE_DATA_LEN);
            }
            else if (decode[2])
            {
                audio_write_and_wait(thiz, mixers[2]->downlink_decode_out, OPUS_ENCODE_DATA_LEN);
            }

            bool need_decode_gain = false;
            for (int i = 0; i < MAX_MIXER_NUM; i++)
            {
                if (decode[i])
                {
                    rt_enter_critical();
                    rt_slist_remove(&mixers[i]->downlink_decode_busy, decode[i]);
                    rt_slist_append(&mixers[i]->downlink_decode_idle, decode[i]);
                    if (rt_slist_first(&mixers[i]->downlink_decode_busy))
                    {
                        need_decode_gain = true;
                    }
                    rt_exit_critical();
                }
            }
            if (need_decode_gain)
                rt_event_send(thiz->event, OPUS_EVENT_BLE_DOWNLINK);

        }

    }
    thiz->is_inited = 0;
    audio_close(thiz->client);

    rt_ringbuffer_destroy(thiz->rb_opus_encode_input);

    if (thiz->encoder)
        opus_encoder_destroy(thiz->encoder);

    rt_event_delete(thiz->event);
    for (int m = 0; m < MAX_MIXER_NUM; m++)
    {
        ble_mixer_data_t *mixer = &thiz->downlink_mixers[m];
        if (mixer->decoder)
        {
            opus_decoder_destroy(mixer->decoder);
            mixer->decoder = NULL;
        }

        for (int i = 0; i < OPUS_DOWNLINK_QUEUE_NUM; i++)
        {
            if (mixer->downlink_queue[i].data)
            {
                rt_free(mixer->downlink_queue[i].data);
                mixer->downlink_queue[i].data = NULL;
            }
        }
    }
    rt_kprintf("---opus test exit---\r\n");
}

int talk_init(audio_rwflag_t flag)
{
    if (g_talker)
    {
        return -1;
    }
    ble_talker_t *thiz;
    g_talker = rt_calloc(1, sizeof(ble_talker_t));
    RT_ASSERT(g_talker);
    thiz = g_talker;
    thiz->flag = flag;
    if (flag & AUDIO_TX)
    {
        thiz->is_rx_enable = 1;
    }
    if (flag & AUDIO_RX)
    {
        thiz->is_tx_enable = 1;
    }

    thiz->is_inited = 0;
    /*
        may need two thread:
            decode thread and encode thread
    */
    thiz->opus_thread = os_thread_create(TALK_THREAD_NAME, opus_thread_entry, thiz,
                                         g_opus_stack, sizeof(g_opus_stack),
                                         OPUS_THREAD_PRIORITY,
                                         RT_THREAD_TICK_DEFAULT);
    RT_ASSERT(thiz->opus_thread);
    return 0;
}

int talk_deinit(void)
{
    if (g_talker)
    {
        ble_talker_t *thiz = g_talker;
        thiz->is_exit = 1;
        while (rt_thread_find(TALK_THREAD_NAME))
        {
            LOG_I("wait thread %s eixt", TALK_THREAD_NAME);
            os_delay(100);
        }
        os_thread_delete(thiz->opus_thread);
        /*notice,
            should close ble first, ble_talk_downlink() may use g_talker
         */
        g_talker = NULL;
        rt_free(thiz);
    }
    return 0;
}

static int ble_talk(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "start") == 0)
        {
            if (argc > 2)
            {
                if (!strcmp(argv[2], "rx"))
                {
                    /*rx ble and to local AUDIO_TX*/
                    return talk_init(AUDIO_TX);
                }
                if (!strcmp(argv[2], "tx"))
                {
                    /*local mic AUDIO_RX and tx tol ble*/
                    return talk_init(AUDIO_RX);
                }
            }
            return talk_init(AUDIO_TXRX);
        }
        else if (strcmp(argv[1], "stop") == 0)
        {
            if (!g_talker)
            {
                return 0;
            }

            ble_talker_t *thiz = g_talker;

            LOG_I("!!!!\nis ble closed?\n");
            LOG_I("ble may still call ble_talk_downlink(), may use invalid g_talker");
            if (argc > 2)
            {
                if (!strcmp(argv[2], "rx"))
                {
                    LOG_I("talk disable rx");
                    thiz->is_rx_enable = 0;
                }
                if (!strcmp(argv[2], "tx"))
                {
                    LOG_I("talk disable tx");
                    thiz->is_tx_enable = 0;
                }
            }
            else
            {
                thiz->is_rx_enable = 0;
                thiz->is_tx_enable = 0;
            }
            if (!thiz->is_rx_enable && !thiz->is_tx_enable)
            {
                LOG_I("talk disable txrx");
                return talk_deinit();
            }
        }
        else
        {
            LOG_I("!!!cmd error?\n example: ble_talk start tx\nexample: ble_talk start rx\nble_talk_stop\n");
        }
    }

    return 0;
}
MSH_CMD_EXPORT(ble_talk, ble talk);

#endif

