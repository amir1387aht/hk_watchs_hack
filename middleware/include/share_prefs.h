/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SHARE_PREFS_H
#define SHARE_PREFS_H


#include <rtthread.h>
#include <stdint.h>
#include <string.h>

/**
 ****************************************************************************************
* @addtogroup shard_pref Shared preference
* @ingroup middleware
* @brief Sifli Shared preference API
* @{
****************************************************************************************
*/


#ifdef __cplusplus
extern "C" {
#endif

#define SHARE_PREFS_MAX_NAME_LEN 32

/// Preference data type
typedef struct
{
    char prfs_name[SHARE_PREFS_MAX_NAME_LEN]; /*!< Preference name*/
    uint32_t mode;                            /*!< Preference mode*/
} share_prefs_t;

/// Preference mode enumeration.
typedef enum
{
    SHAREPREFS_MODE_PRIVATE = 0x1,              /*!< Private */
    SHAREPREFS_MODE_WORLD_READABLE = 0x2,       /*!< Readable */
    SHAREPREFS_MODE_WORLD_WRITEABLE = 0x4,      /*!< Writable */
} share_prefs_mode;

///share_prefs api

/**
    @brief Open shared preference database
    @param[in] prfs_name Preference name
    @param[in] mode      Mode to open
    @retval Handle of shared preference database
*/
share_prefs_t *share_prefs_open(const char *prfs_name, uint32_t mode);
/**
    @brief Close shared preference database
    @param[in] prfs Handle of shared preference database
    @retval RT_EOK if successful, otherwise return error number <0
*/
rt_err_t share_prefs_close(share_prefs_t *prfs);
/**
    @brief Clear shared preference database
    @param[in] prfs Handle of shared preference database
    @retval RT_EOK if successful, otherwise return error number <0
*/
rt_err_t share_prefs_clear(share_prefs_t *prfs);

/**
    @brief Remove an entry for a preference database
    @param[in] prfs Handle of shared preference database
    @param[in] key Key name of preference entry
    @retval RT_EOK if successful, otherwise return error number <0
*/
rt_err_t share_prefs_remove(share_prefs_t *prfs, const char *key);

//int
/**
    @brief Get a integer type of a preference
    @param[in] prfs Handle of shared preference database
    @param[in] key Key name of preference entry
    @param[in] default_v Default value if preference not found    .
    @retval value of interger
*/
int32_t share_prefs_get_int(share_prefs_t *prfs, const char *key, int32_t default_v);

/**
    @brief Set a integer type of a preference
    @param[in] prfs Handle of shared preference database
    @param[in] key Key name of preference entry
    @param[in] value Value to be set
    @retval RT_EOK if successful, otherwise return error number <0
*/rt_err_t share_prefs_set_int(share_prefs_t *prfs, const char *key, int32_t value);

//string
/**
    @brief Get a string type of a preference
    @param[in] prfs Handle of shared preference database
    @param[in] key Key name of preference entry
    @param[in,out] buf Data buffer of preference value
    @param[in] buf_len Max buffer length
    @retval Length of buf if successful, otherwise return error number <0
*/
int32_t share_prefs_get_string(share_prefs_t *prfs, const char *key, char *buf, int32_t buf_len);
/*
    set string
*/
/**
    @brief Set a string type of a preference
    @param[in] prfs Handle of shared preference database
    @param[in] key Key name of preference entry
    @param[in] buf Data buffer of preference value
    @retval RT_EOK if successful, otherwise return error number <0
*/
rt_err_t share_prefs_set_string(share_prefs_t *prfs, const char *key, const char *buf);

//block
/**
    @brief Get a general block type of a preference
    @param[in] prfs Handle of shared preference database
    @param[in] key Key name of preference entry
    @param[in,out] buf Data buffer of preference value
    @param[in] buf_len Max buffer length
    @retval Length of buf if successful, otherwise return error number <0
*/
int32_t share_prefs_get_block(share_prefs_t *prfs, const char *key, void *buf, int32_t buf_len);

/**
    @brief Set a general block type of a preference
    @param[in] prfs Handle of shared preference database
    @param[in] key Key name of preference entry
    @param[in] buf Data buffer of preference value
    @param[in] buf_len  Buffer length
    @retval RT_EOK if successful, otherwise return error number <0
*/
rt_err_t share_prefs_set_block(share_prefs_t *prfs, const char *key, const void *buf, int32_t buf_len);

/// @} shard_pref
/// @} file

#ifdef __cplusplus
}
#endif
#endif

