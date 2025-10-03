#ifndef FDB_H
#define FDB_H

#include "flashdb.h"
#include <dfs_posix.h>
#include <rtthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef FDB_USING_KVDB
    #define APP_KVDB_NAME "fdb_kvdb"
    #define APP_KVDB_PATH "/sys/db/fdb_kvdb"
    #define APP_KVDB_MAX (0x4000)    /* 16KB */

    extern struct fdb_kvdb g_kvdb_db;
    extern fdb_kvdb_t p_kvdb_db;
    extern struct rt_mutex g_kvdb_db_mutex;

    #if (FDB_KV_CACHE_TABLE_SIZE == 1)
    extern uint32_t g_ble_db_cache[256];
    #endif
#endif

#ifdef FDB_USING_TSDB
    #define APP_TSDB_NAME "fdb_tsdb"
    #define APP_TSDB_PATH "/sys/db/fdb_tsdb"
    #define APP_TSDB_MAX (0x4000)    /* 16KB */
    #define APP_TSDB_MAX_DATA_LEN (256)

    extern struct fdb_tsdb g_tsdb_db;
    extern fdb_tsdb_t p_tsdb_db;
#endif

    /* FlashDB initialization */
    int flashdb_init(void);

    /* KV APIs */
    bool kv_set_int(const char *key, int value);
    int kv_get_int(const char *key, int default_value);
    bool kv_set_float(const char *key, float value);
    float kv_get_float(const char *key, float default_value);
    bool kv_set_bool(const char *key, bool value);
    bool kv_get_bool(const char *key, bool default_value);
    bool kv_set_string(const char *key, const char *value);
    const char *kv_get_string(const char *key, const char *default_val);
    bool kv_del(const char *key);

    /* TSDB APIs */
    bool ts_add(const char *value);
    void ts_print_query_by_time(fdb_time_t from, fdb_time_t to);
    void ts_query_by_time(fdb_tsl_cb cb, fdb_time_t from, fdb_time_t to);
    void ts_print_all_query();
    void ts_query_all(fdb_tsl_cb cb);
    size_t ts_count_get(fdb_time_t from, fdb_time_t to);
    size_t ts_count_all();
    void ts_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* FDB_H */