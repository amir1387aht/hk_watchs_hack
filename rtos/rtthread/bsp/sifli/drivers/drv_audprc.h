/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __DRV_AUDIO_PRC_H__
#define __DRV_AUDIO_PRC_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <drv_common.h>

#define MUTE_UNDER_MIN_VOLUME  (-256)

int rt_bf0_audio_prc_init(void);
uint8_t bf0_audprc_get_tx_rbf_en();
uint8_t bf0_audprc_get_tx_channel();
void bf0_audprc_set_tx_channel(uint8_t chan);

uint8_t bf0_audprc_get_rx_channel();
void bf0_audprc_set_rx_channel(uint8_t chan);

void bf0_audprc_device_write(rt_device_t dev, rt_off_t    pos, const void *buffer, rt_size_t   size);

void bf0_audprc_dma_restart(uint16_t chann_used);
int eq_get_music_volumex2(uint8_t level);
int eq_get_tel_volumex2(uint8_t level);
int8_t eq_get_decrease_level(int is_tel, int volumex2);
uint8_t eq_is_working();
int eq_get_default_volumex2();
void bf0_audprc_eq_enable_offline(uint8_t is_enable);
void bf0_audprc_stop();
void mic_gain_decrease(int8_t db);

#endif

