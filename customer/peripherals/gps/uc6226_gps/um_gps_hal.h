/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GPS_HAL_H_
#define _GPS_HAL_H_


#include "gps_types.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int             prn;
    float           cno;
    float           elevation; /* degrees */
    float           azimuth; /* degrees */
} gpsSvInfo_t;

typedef struct
{
    uint16_t             num_svs;
    gpsSvInfo_t     sv_list[GPS_MAX_SVS];
    uint32_t             ephemeris_mask;
    uint32_t             almanac_mask;
    uint32_t             used_in_fix_mask;
} gpsSvStatus_t;

typedef struct
{
    uint16_t             num_svs_gln;
    gpsSvInfo_t     gln_sv_list[GLN_MAX_SVS];
    uint32_t             gln_ephemeris_mask;
    uint32_t             gln_almanac_mask;
    uint32_t             gln_used_in_fix_mask;
} glnSvStatus_t;

typedef struct
{
    gpsSvStatus_t gps_sv_status;
    glnSvStatus_t gln_sv_status;
} gnssSvStatus_t;

typedef struct
{
    double             utcTime;
    uint16_t             msgLen;
    int8_t              nmeaInfo[GPS_NMEA_STRING_MAXLEN];
} gpsNmea_t;

enum
{
    GPS_HSM_STARTMODE_NORMAL,
    GPS_HSM_STARTMODE_TEST_COLD,
    GPS_HSM_STARTMODE_TEST_HOT,
    GPS_HSM_STARTMODE_TEST_WARM,
};

/*
 * Android status
 */
typedef enum
{
    GPS_HAL_STATUS_ON,
    GPS_HAL_STATUS_OFF,
} gpsHsmStatus_t;

enum
{
    GPS_HAL_STATE_INVALID  = 0x00000001,
    GPS_HAL_STATE_DESTROY  = 0x00000010,
    GPS_HAL_STATE_INIT     = 0x00000100,
    GPS_HAL_STATE_STARTED  = 0x00001000,
    GPS_HAL_STATE_STOPPED  = 0x00010000,
};

int gps_send_cmd(uint8_t type, char *cmd, uint32_t len);
int gps_hal_init(void);
int gps_hal_start(GpsCallbacks *callbacks);
int gps_hal_stop(void);
uint32_t gps_hal_set_state(uint32_t state);
uint32_t gps_hal_get_state();
int gps_status_update(gpsHsmStatus_t hsmStatus);
int gps_hal_update_svstatus(GnssSvInfo sv_list[]);
int gps_hal_update_location(GpsLocation fix);
int gps_hal_update_nmea(char *pbuf);
int gps_hal_cleanup(void);
int gps_hal_set_local_baud(int baud);
uint32_t gps_hal_get_uart(void);

#ifdef __cplusplus
}
#endif
#endif

