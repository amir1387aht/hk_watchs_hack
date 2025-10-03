#include "./datetime_utils.h"

#define KEY_LAST_DATETIME "last_datetime"

static bool get_current_datetime(int *year, int *month, int *day, int *hour,
                                 int *minute, int *second)
{
    time_t now = time(NULL);
    if (now == (time_t)(-1))
    {
        return false;
    }

    struct tm *tm_now = localtime(&now);
    if (tm_now == NULL)
    {
        return false;
    }

    *year = tm_now->tm_year + 1900;
    *month = tm_now->tm_mon + 1;
    *day = tm_now->tm_mday;
    *hour = tm_now->tm_hour;
    *minute = tm_now->tm_min;
    *second = tm_now->tm_sec;

    return true;
}

void datetime_save(int year, int month, int day, int hour, int minute,
                   int second)
{
    char datetime_str[32];
    snprintf(datetime_str, sizeof(datetime_str),
             "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute,
             second);

    if (!kv_set_string(KEY_LAST_DATETIME, datetime_str))
    {
        rt_kprintf("[datetime_utils] Failed to save datetime!\n");
    }
    else
    {
        rt_kprintf("[datetime_utils] Saved datetime: %s\n", datetime_str);
    }
}

bool datetime_load(void)
{
    const char *datetime_str = kv_get_string(KEY_LAST_DATETIME, NULL);

    if (datetime_str == NULL)
    {
        rt_kprintf("[datetime_utils] No saved datetime found.\n");
        return false;
    }

    int year, month, day, hour, minute, second;
    if (sscanf(datetime_str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour,
               &minute, &second) == 6)
    {
        set_date(year, month, day);
        set_time(hour, minute, second);

        rt_kprintf("[datetime_utils] Loaded datetime: %s\n", datetime_str);
        return true;
    }
    else
    {
        rt_kprintf("[datetime_utils] Invalid datetime format in DB: %s\n",
                   datetime_str);
        return false;
    }
}

bool datetime_save_current(void)
{
    int year, month, day, hour, minute, second;

    if (!get_current_datetime(&year, &month, &day, &hour, &minute, &second))
    {
        rt_kprintf("[datetime_utils] Failed to get current datetime.\n");
        return false;
    }

    datetime_save(year, month, day, hour, minute, second);
    return true;
}