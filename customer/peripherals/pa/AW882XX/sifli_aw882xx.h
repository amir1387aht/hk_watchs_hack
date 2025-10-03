/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SILFI_AW882XX_H
#define __SILFI_AW882XX_H

#ifdef __cplusplus
extern "C" {
#endif

void sifli_aw882xx_start(uint32_t samplerate, uint8_t chn_sel);
void sifli_aw882xx_stop();
int rt_aw882xx_init();

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __SILFI_AW882XX_H */

