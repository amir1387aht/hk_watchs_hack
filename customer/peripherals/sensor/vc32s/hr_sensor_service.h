/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HR_SENSOR_SERVICE_H_
#define _HR_SENSOR_SERVICE_H_


#include "board.h"
#include "sensor.h"
#include "vcHr02Hci.h"
#if defined (HR_ALGO_USING_GSENSOR_DATA)
    #include "gsensor_service.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define PPG_BUFF_SIZE           40              //ppg buff szie
#define HR_PERIOD_TIMER         800     //vc32s every  800ms get fifo data
#define HR_ALGO_PEROID          800

typedef struct
{
    uint8_t status;
    uint8_t hr;
    uint8_t bp_l;
    uint8_t bp_h;
    uint8_t spo2;
} hr_algo_result_t;

typedef struct
{
    uint8_t hr_id;
    uint8_t type;
} hr_sensor_info_t;

/* hr device structure */
struct hr_sensor_device
{
    rt_device_t bus;
    rt_uint8_t id;
    rt_uint8_t i2c_addr;
};

typedef struct
{
    bool            vcFifoReadFlag;
    bool            vcPsFlag;
    uint8_t         wearstatus;
    uint8_t         envValue[3];
    uint8_t         SampleRate;
    int16_t         ppgValue[PPG_BUFF_SIZE];
} hr_raw_data_t;

rt_err_t vc32s_int_open(void);
void vc32s_int_close(void);
void vc32sStart(vcHr02_t *pVcHr02, vcHr02Mode_t vcHr02WorkMode);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_GOODIX_GH3011_H__

