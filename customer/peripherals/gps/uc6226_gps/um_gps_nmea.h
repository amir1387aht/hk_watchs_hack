/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GPS_NMEA_H_
#define _GPS_NMEA_H_


#include "gps_types.h"


#ifdef __cplusplus
extern "C" {
#endif

enum
{
    GNSS_MAX_SVS_COUNT = 64u, // 64
};

enum
{
    NMEA_GGA = 0x0001,
    NMEA_GLL = 0x0002,
    NMEA_GSA = 0x0004,
    NMEA_GSV = 0x0008,
    NMEA_RMC = 0x0010,
    NMEA_VTG = 0x0020,
    NMEA_ZDA = 0x0040,
    NAV_POS  = 0x0080,
    NAV_VEL  = 0x0100,
    NAV_TIME = 0x0200,
    NAV_ACC  = 0x0400,
};

enum
{
    GNSS_SV_FLAGS_NONE = 0,
    GNSS_SV_FLAGS_HAS_EPHEMERIS_DATA = 1, // (1 << 0)
    GNSS_SV_FLAGS_HAS_ALMANAC_DATA = 2, // (1 << 1)
    GNSS_SV_FLAGS_USED_IN_FIX = 4, // (1 << 2)
    GNSS_SV_FLAGS_HAS_CARRIER_FREQUENCY = 8, // (1 << 3)
};

enum
{
    GPS_LOCATION_HAS_LAT_LONG = 1, // 0x0001
    GPS_LOCATION_HAS_ALTITUDE = 2, // 0x0002
    GPS_LOCATION_HAS_SPEED = 4, // 0x0004
    GPS_LOCATION_HAS_BEARING = 8, // 0x0008
    GPS_LOCATION_HAS_HORIZONTAL_ACCURACY = 16, // 0x0010
    GPS_LOCATION_HAS_VERTICAL_ACCURACY = 32, // 0x0020
    GPS_LOCATION_HAS_SPEED_ACCURACY = 64, // 0x0040
    GPS_LOCATION_HAS_BEARING_ACCURACY = 128, // 0x0080
};

#define GPS_LOCATION_HAS_ACCURACY GPS_LOCATION_HAS_HORIZONTAL_ACCURACY

/* commands sent to the gps thread */
enum
{
    POS_MODE_GPS = 1,
    POS_MODE_BD = 2,
    POS_MODE_GN = 3,
};

typedef struct
{
    int valid;
    double systime;
    GpsUtcTime timestamp;
} UmTimemap_t;

typedef struct
{
    int     pos;
    int     overflow;
    struct tm utc_time;
    GpsLocation  fix;
    GnssSvInfo sv_list[MAX_SVS];
    GnssSvInfo sv_list_tmp[MAX_SVS];
    int     sv_status_changed;
    int     sv_status_commit;
    char    in[NMEA_MAX_SIZE];
    int     gsa_fixed;
    UmTimemap_t timemap;
    int     nmea_mode;
} NmeaReader;

void nmea_reader_init();
void um_gps_nmea_parse(char *buf, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif /* _GPS_NMEA_H_ */

