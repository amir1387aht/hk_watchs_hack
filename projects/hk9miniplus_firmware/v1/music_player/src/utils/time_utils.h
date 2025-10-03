#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#include <time.h>
#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C"
{
#endif
    /// Get current system time as struct tm
    int time_utils_get_tm(struct tm *out_tm);

    /// Get formatted time string (e.g., "xxxx-xx-xx xx:xx:xx")
    const char *time_utils_get_str(void);

    // Gets Short Big Time Like TUE 22 JUL
    const char *date_utils_get_str(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIME_UTILS_H__ */
