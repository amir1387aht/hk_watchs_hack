#ifndef DATETIME_UTILS_H
#define DATETIME_UTILS_H

#include <stdbool.h>
#include "flashdb/fdb.h"
#include <rtthread.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief Save the given date and time into FlashDB KV.
 */
void datetime_save(int year, int month, int day, int hour, int minute,
                   int second);

/**
 * @brief Load the last saved date and time from FlashDB KV.
 *        If successful, calls set_date() and set_time().
 *
 * @return true if datetime loaded successfully, false if not found or invalid.
 */
bool datetime_load(void);

/**
 * @brief Automatically save the current system/RTC datetime into FlashDB KV.
 *
 * @return true if successfully saved, false otherwise.
 */
bool datetime_save_current(void);

#endif /* DATETIME_UTILS_H */