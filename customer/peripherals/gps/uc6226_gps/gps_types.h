/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GPS_TYPES_H_
#define _GPS_TYPES_H_




#ifdef __cplusplus
extern "C" {
#endif

#define GNSS_MAX_SVS 128

#define GPS_NMEA_STRING_MAXLEN 1024

#define GPS_MAX_SVS 32
#define GLN_MAX_SVS 24

#define  MAX_SVS 256
#define  NMEA_MAX_SIZE  1024

enum
{
    GNSS_CONSTELLATION_UNKNOWN = 0,
    GNSS_CONSTELLATION_GPS = 1,
    GNSS_CONSTELLATION_SBAS = 2,
    GNSS_CONSTELLATION_GLONASS = 3,
    GNSS_CONSTELLATION_QZSS = 4,
    GNSS_CONSTELLATION_BEIDOU = 5,
    GNSS_CONSTELLATION_GALILEO = 6,
};

enum
{
    GPS_STATUS_NONE = 0,
    GPS_STATUS_SESSION_BEGIN = 1,
    GPS_STATUS_SESSION_END = 2,
    GPS_STATUS_ENGINE_ON = 3,
    GPS_STATUS_ENGINE_OFF = 4,
};

typedef struct
{
    /** set to sizeof(GpsStatus) */
    size_t          size;
    uint16_t        status;
} GpsStatus;

/*
 * Milliseconds since January 1, 1970
 */
typedef int64_t GpsUtcTime;
/*
 * Represents a location.
 */
typedef struct
{
    /*
     * set to sizeof(GpsLocation)
     */
    size_t          size;
    /*
     * Contains GpsLocationFlags bits.
     */
    uint16_t        flags;
    /*
     * Represents latitude in degrees.
     */
    double          latitude;
    /*
     * Represents longitude in degrees.
     */
    double          longitude;
    /**
      * Represents altitude in meters above the WGS 84 reference ellipsoid.
      */
    double          altitude;
    /** Represents speed in meters per second. */
    float           speed;
    /** Represents heading in degrees. */
    float           bearing;
    /** Represents expected accuracy in meters. */
    float           accuracy;
    /** Timestamp for the location fix. */
    GpsUtcTime      timestamp;
} GpsLocation;

/** * Constellation type of GnssSvInfo */
typedef uint8_t                         GnssConstellationType;
typedef uint8_t                                 GnssSvFlags;

typedef struct
{
    /** set to sizeof(GnssSvInfo) */
    size_t size;
    /**
    * Pseudo-random number for the SV, or FCN/OSN number for Glonass. The
    * distinction is made by looking at constellation field. Values should be
    * in the range of:
    *
    * - GPS:     1-32
    * - SBAS:    120-151, 183-192
    * - GLONASS: 1-24, the orbital slot number (OSN), if known.  Or, if not:
    *            93-106, the frequency channel number (FCN) (-7 to +6) offset by + 100
    *            i.e. report an FCN of -7 as 93, FCN of 0 as 100, and FCN of +6 as 106.
    * - QZSS:    193-200
    * - Galileo: 1-36
    * - Beidou:  1-37     */
    int16_t svid;
    /**
    * Defines the constellation of the given SV. Value should be one of those
    * GNSS_CONSTELLATION_* constants
    */
    GnssConstellationType constellation;
    /**     * Carrier-to-noise density in dB-Hz, typically in the range [0, 63].
    * It contains the measured C/N0 value for the signal at the antenna port.
    *     * This is a mandatory value.
    */
    float c_n0_dbhz;
    /** Elevation of SV in degrees. */
    float elevation;
    /** Azimuth of SV in degrees. */
    float azimuth;
    /**     * Contains additional data about the given SV. Value should be one of those     * GNSS_SV_FLAGS_* constants     */
    GnssSvFlags flags;
} GnssSvInfo;

/**
 * Represents SV status.
 */
typedef struct
{
    /** set to sizeof(GnssSvStatus) */
    size_t size;

    /** Number of GPS SVs currently visible, refers to the SVs stored in sv_list */
    int num_svs;
    /**
     * Pointer to an array of SVs information for all GNSS constellations,
     * except GPS, which is reported using sv_list
     */
    GnssSvInfo gnss_sv_list[GNSS_MAX_SVS];

} GnssSvStatus;

/**
 * Callback with location information.
 */
typedef void (* gps_location_callback)(GpsLocation *location);

/**
 * Callback with status information.
 */
typedef void (* gps_status_callback)(GpsStatus *status);

/**
 * Callback with SV status information.
 */
typedef void (* gnss_sv_status_callback)(GnssSvStatus *sv_info);
/**
 * Callback for reporting NMEA sentences.
 */
typedef void (* gps_nmea_callback)(GpsUtcTime timestamp, const char *nmea, int length);

typedef struct
{
    /** set to sizeof(GpsCallbacks) */
    size_t      size;
    gps_location_callback location_cb;
    gps_status_callback status_cb;
    gnss_sv_status_callback gnss_sv_status_cb;
    gps_nmea_callback nmea_cb;
} GpsCallbacks;


#ifdef __cplusplus
}
#endif
#endif /* _GPS_TYPES_H_ */

