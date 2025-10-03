#include "./fdb.h"

/* Database instances */
#ifdef FDB_USING_KVDB
struct fdb_kvdb g_kvdb_db;
fdb_kvdb_t p_kvdb_db = &g_kvdb_db;
struct rt_mutex g_kvdb_db_mutex;
    #if (FDB_KV_CACHE_TABLE_SIZE == 1)
uint32_t g_ble_db_cache[256];
    #endif
#endif

#ifdef FDB_USING_TSDB
struct fdb_tsdb g_tsdb_db;
fdb_tsdb_t p_tsdb_db = &g_tsdb_db;
#endif

/* Default KV values and nodes */
static const char model_name_value[] = "HK10 Ultra 3";
static const char version_value[] = "1.0.0";

static struct fdb_default_kv_node default_kv_nodes[] = {
    {"model_name", model_name_value, sizeof(model_name_value)},
    {"version", version_value, sizeof(version_value)}};

static struct fdb_default_kv default_kv = {
    .kvs = default_kv_nodes,
    .num = sizeof(default_kv_nodes) / sizeof(default_kv_nodes[0]),
};

/* Lock/unlock callbacks */
static void _lock(fdb_db_t db)
{
    rt_mutex_take(&g_kvdb_db_mutex, RT_WAITING_FOREVER);
}

static void _unlock(fdb_db_t db)
{
    rt_mutex_release(&g_kvdb_db_mutex);
}

/* Time callback */
static fdb_time_t _get_time(void)
{
    time_t t = 0;

    time(&t);
    return t;
}

int flashdb_init(void)
{
    fdb_err_t err;

    rt_kprintf("[FlashDB] Starting initialization...\n");
    
    /* Ensure folders exsist */
    mkdir(APP_KVDB_PATH, 755);
    mkdir(APP_TSDB_PATH, 755);

    /* mutex initialization. */
    rt_mutex_init(&g_kvdb_db_mutex, "kvdb_mtx", RT_IPC_FLAG_FIFO);
    rt_kprintf("[FlashDB] Mutex initialized\n");

    /* Try different configurations */
    const char *path = APP_KVDB_PATH;
    int sec_size = PKG_FLASHDB_ERASE_GRAN;
    int max_size = APP_KVDB_MAX;
    bool file_mode = true;

    rt_kprintf("[FlashDB] KVDB Config:\n");
    rt_kprintf("  Path: %s\n", path);
    rt_kprintf("  Sector Size: %d\n", sec_size);
    rt_kprintf("  Max Size: %d (0x%X)\n", max_size, max_size);
    rt_kprintf("  File Mode: %s\n", file_mode ? "true" : "false");

    /* Try setting parameters before init */
    fdb_kvdb_control(p_kvdb_db, FDB_KVDB_CTRL_SET_SEC_SIZE, (void *)&sec_size);
    fdb_kvdb_control(p_kvdb_db, FDB_KVDB_CTRL_SET_MAX_SIZE, (void *)&max_size);
    fdb_kvdb_control(p_kvdb_db, FDB_KVDB_CTRL_SET_FILE_MODE,
                     (void *)&file_mode);

    /* Initialize with NULL default_kv first to see if that works */
    err = fdb_kvdb_init(p_kvdb_db, APP_KVDB_NAME, path, &default_kv, NULL);

    if (err != FDB_NO_ERR)
    {
        rt_kprintf("[FlashDB] KVDB Init Failed with error: %d\n", err);
        return RT_ERROR;
    }

    rt_kprintf("[FlashDB] KVDB initialized successfully\n");

    /* Now set lock/unlock after successful init */
    fdb_kvdb_control(p_kvdb_db, FDB_KVDB_CTRL_SET_LOCK, _lock);
    fdb_kvdb_control(p_kvdb_db, FDB_KVDB_CTRL_SET_UNLOCK, _unlock);

    /* TSDB initialization */
    path = APP_TSDB_PATH;
    max_size = APP_TSDB_MAX;

    rt_kprintf("[FlashDB] TSDB Config:\n");
    rt_kprintf("  Path: %s\n", path);
    rt_kprintf("  Max Size: %d\n", max_size);

    fdb_tsdb_control(p_tsdb_db, FDB_TSDB_CTRL_SET_SEC_SIZE, (void *)&sec_size);
    fdb_tsdb_control(p_tsdb_db, FDB_TSDB_CTRL_SET_MAX_SIZE, (void *)&max_size);
    fdb_tsdb_control(p_tsdb_db, FDB_TSDB_CTRL_SET_FILE_MODE,
                     (void *)&file_mode);

    err = fdb_tsdb_init(p_tsdb_db, APP_TSDB_NAME, path, _get_time,
                        APP_TSDB_MAX_DATA_LEN, NULL);

    if (err != FDB_NO_ERR)
    {
        rt_kprintf("[FlashDB] TSDB Init Failed with error: %d\n", err);
        return RT_ERROR;
    }

    rt_kprintf("[FlashDB] TSDB initialized successfully\n");
    rt_kprintf("[FlashDB] All initialization complete\n");

    return RT_EOK;
}

/* KV wrappers with debug */
bool kv_set_int(const char *key, int value)
{
    struct fdb_blob blob = {0};
    fdb_blob_make(&blob, &value, sizeof(value));

    fdb_err_t err = fdb_kv_set_blob(p_kvdb_db, key, &blob);
    if (err != FDB_NO_ERR)
    {
        rt_kprintf("[KV] Failed to set int key '%s', error: %d\n", key, err);
    }
    return err == FDB_NO_ERR;
}

int kv_get_int(const char *key, int def)
{
    int v = def;
    struct fdb_blob blob = {0};
    fdb_blob_make(&blob, &v, sizeof(v));

    fdb_err_t err = fdb_kv_get_blob(p_kvdb_db, key, &blob);

    if (err != FDB_NO_ERR && err != FDB_KV_NAME_ERR)
        rt_kprintf("[KV] Error getting int key '%s': %d\n", key, err);

    return v;
}

bool kv_set_float(const char *key, float value)
{
    struct fdb_blob blob = {0};
    fdb_blob_make(&blob, &value, sizeof(value));

    fdb_err_t err = fdb_kv_set_blob(p_kvdb_db, key, &blob);

    if (err != FDB_NO_ERR)
        rt_kprintf("[KV] Failed to set float key '%s', error: %d\n", key, err);

    return err == FDB_NO_ERR;
}

float kv_get_float(const char *key, float def)
{
    float v = def;
    struct fdb_blob blob = {0};
    fdb_blob_make(&blob, &v, sizeof(v));

    fdb_err_t err = fdb_kv_get_blob(p_kvdb_db, key, &blob);

    if (err != FDB_NO_ERR && err != FDB_KV_NAME_ERR)
        rt_kprintf("[KV] Error getting float key '%s': %d\n", key, err);

    return v;
}

bool kv_set_bool(const char *key, bool value)
{
    struct fdb_blob blob = {0};
    fdb_blob_make(&blob, &value, sizeof(value));

    fdb_err_t err = fdb_kv_set_blob(p_kvdb_db, key, &blob);

    if (err != FDB_NO_ERR)
        rt_kprintf("[KV] Failed to set bool key '%s', error: %d\n", key, err);

    return err == FDB_NO_ERR;
}

bool kv_get_bool(const char *key, bool def)
{
    bool v = def;
    struct fdb_blob blob = {0};
    fdb_blob_make(&blob, &v, sizeof(v));

    fdb_err_t err = fdb_kv_get_blob(p_kvdb_db, key, &blob);

    if (err != FDB_NO_ERR && err != FDB_KV_NAME_ERR)
        rt_kprintf("[KV] Error getting bool key '%s': %d\n", key, err);

    return v;
}

bool kv_set_string(const char *key, const char *value)
{
    fdb_err_t err = fdb_kv_set(p_kvdb_db, key, value);

    if (err != FDB_NO_ERR)
        rt_kprintf("[KV] Failed to set string key '%s', error: %d\n", key, err);

    return err == FDB_NO_ERR;
}

const char *kv_get_string(const char *key, const char *def)
{
    const char *r = fdb_kv_get(p_kvdb_db, key);

    return r ? r : def;
}

bool kv_del(const char *key)
{
    fdb_err_t err = fdb_kv_del(p_kvdb_db, key);

    if (err != FDB_NO_ERR)
        rt_kprintf("[KV] Failed to delete key '%s', error: %d\n", key, err);

    return err == FDB_NO_ERR;
}

/* TS wrappers */
bool ts_add(const char *value)
{
    struct fdb_blob blob;
    fdb_err_t err = fdb_tsl_append(
        p_tsdb_db, fdb_blob_make(&blob, value, strlen(value) + 1));

    if (err == FDB_SAVED_FULL)
    {
        ts_clear();
        return ts_add(value);
    }
    else if (err != FDB_NO_ERR) rt_kprintf("[TS] Failed to append value, error: %d\n", err);

    return err == FDB_NO_ERR;
}

static bool query_by_time_cb(fdb_tsl_t tsl, void *arg)
{
    struct fdb_blob blob;
    char value[128];
    struct tm *time_info;
    fdb_tsdb_t db = arg;
    time_t t = 0;

    fdb_blob_read(
        (fdb_db_t)db,
        fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, value, sizeof(value))));

    t = tsl->time;
    time_info = localtime(&t);

    rt_kprintf("[query_by_time_cb] queried a TSL: value: %s time: %d %s", value,
               (int)tsl->time, asctime(time_info));

    return false;
}

void ts_print_query_by_time(fdb_time_t from, fdb_time_t to)
{
    ts_query_by_time(query_by_time_cb, from, to);
}

void ts_query_by_time(fdb_tsl_cb cb, fdb_time_t from, fdb_time_t to)
{
    fdb_tsl_iter_by_time(p_tsdb_db, from, to, cb, p_tsdb_db);
}

static bool query_cb(fdb_tsl_t tsl, void *arg)
{
    struct fdb_blob blob;
    char value[128];
    struct tm *time_info;
    fdb_tsdb_t db = arg;
    time_t t = 0;

    fdb_blob_read(
        (fdb_db_t)db,
        fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, value, sizeof(value))));

    t = tsl->time;
    time_info = localtime(&t);

    rt_kprintf("[ts_query_cb] queried a TSL: value: %s time: %d %s", value,
               (int)tsl->time, asctime(time_info));

    return false;
}

void ts_print_all_query()
{
    ts_query_all(query_cb);
}

void ts_query_all(fdb_tsl_cb cb)
{
    fdb_tsl_iter(p_tsdb_db, cb, p_tsdb_db);
}

size_t ts_count_all()
{
    return ts_count_get(0, 0x7FFFFFFF);
}

size_t ts_count_get(fdb_time_t from, fdb_time_t to)
{
    return fdb_tsl_query_count(p_tsdb_db, from, to, FDB_TSL_WRITE);
}

void ts_clear(void)
{
    fdb_tsl_clean(p_tsdb_db);
}

/* MSH Wrappers */
static int msh_ts_add(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("Usage: %s \"log_string\"\n", argv[0]);
        return -RT_ERROR;
    }

    if (ts_add(argv[1]))
    {
        rt_kprintf("Log added: \"%s\"\n", argv[1]);
        return RT_EOK;
    }
    else
    {
        rt_kprintf("Failed to add log\n");
        return -RT_ERROR;
    }
}
MSH_CMD_EXPORT(msh_ts_add, ts_add<string> : append a log entry to TSDB);

static int msh_ts_print_query_by_time(int argc, char **argv)
{
    if (argc != 3)
    {
        rt_kprintf("Usage: %s <from_time> <to_time>\n", argv[0]);
        return -RT_ERROR;
    }

    /* parse arguments */
    long from = atol(argv[1]);
    long to = atol(argv[2]);
    if (from > to)
    {
        rt_kprintf("Error: from_time must be <= to_time\n");
        return -RT_ERROR;
    }

    ts_print_query_by_time((fdb_time_t)from, (fdb_time_t)to);
    return RT_EOK;
}
MSH_CMD_EXPORT(msh_ts_print_query_by_time, ts_print_query_by_time <from> <to>: query logs by time);

static int msh_ts_count_all(int argc, char **argv)
{
    if (argc != 1)
    {
        rt_kprintf("Usage: %s\n", argv[0]);
        return -RT_ERROR;
    }
    size_t cnt = ts_count_all();
    /* ts_count_all() already prints, but we can repeat */
    rt_kprintf("Total logs: %d\n", cnt);
    return RT_EOK;
}
MSH_CMD_EXPORT(msh_ts_count_all, ts_count_all : print count of all logs);

static int msh_ts_clear(int argc, char **argv)
{
    if (argc != 1)
    {
        rt_kprintf("Usage: %s\n", argv[0]);
        return -RT_ERROR;
    }
    ts_clear();
    rt_kprintf("All logs deleted\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(msh_ts_clear, ts_clear : delete all logs);

static int msh_ts_print_all(int argc, char **argv)
{
    if (argc != 1)
    {
        rt_kprintf("Usage: %s\n", argv[0]);
        return -RT_ERROR;
    }
    ts_print_all_query();
    return RT_EOK;
}
MSH_CMD_EXPORT(msh_ts_print_all, ts_print_all_query : print all logs);