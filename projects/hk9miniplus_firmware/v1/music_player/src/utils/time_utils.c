#include "./time_utils.h"

static char time_str_buf[32];

const char *date_utils_get_str(void)
{
    static const char *weekdays[] = {"SUN", "MON", "TUE", "WED",
                                     "THU", "FRI", "SAT"};

    static const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                   "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    time_t now = time(NULL);
    struct tm *tm_ptr = localtime(&now);
    if (!tm_ptr)
        return "Invalid";

    snprintf(time_str_buf, sizeof(time_str_buf), "%s %02d %s",
             weekdays[tm_ptr->tm_wday], tm_ptr->tm_mday,
             months[tm_ptr->tm_mon]);

    return time_str_buf;
}

int time_utils_get_tm(struct tm *out_tm)
{
    if (!out_tm)
        return -1;

    time_t now = time(NULL);
    struct tm *tm_ptr = localtime(&now);
    if (!tm_ptr)
        return -1;

    *out_tm = *tm_ptr;
    return 0;
}

const char *time_utils_get_str(void)
{
    time_t now = time(NULL);
    struct tm *tm_ptr = localtime(&now);
    if (!tm_ptr)
        return "Invalid";

    snprintf(time_str_buf, sizeof(time_str_buf),
             "%04d-%02d-%02d %02d:%02d:%02d", tm_ptr->tm_year + 1900,
             tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour,
             tm_ptr->tm_min, tm_ptr->tm_sec);

    return time_str_buf;
}

static void set_date_cmd(int argc, char **argv)
{
    if (argc != 4)
    {
        rt_kprintf("Usage: set_date <year> <month> <day>\n");
        return;
    }

    set_date(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    rt_kprintf("Date set to: %s\n", time_utils_get_str());
}
MSH_CMD_EXPORT(set_date_cmd, Set system date : set_date<year><month><day>);

static void set_time_cmd(int argc, char **argv)
{
    if (argc != 4)
    {
        rt_kprintf("Usage: set_time <hour> <min> <sec>\n");
        return;
    }

    set_time(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    rt_kprintf("Time set to: %s\n", time_utils_get_str());
}
MSH_CMD_EXPORT(set_time_cmd, Set system time : set_time<hour><min><sec>);
