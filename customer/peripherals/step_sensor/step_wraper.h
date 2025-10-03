/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __VIRT_STEP_SENSOR_HDR_FILE__
#define __VIRT_STEP_SENSOR_HDR_FILE__



#ifdef __cplusplus
extern "C" {
#endif

int virt_step_init(void);

int vstep_get_step();

int vstep_get_distance();


#ifdef __cplusplus
}
#endif
#endif /* __VIRT_STEP_SENSOR_HDR_FILE__*/

