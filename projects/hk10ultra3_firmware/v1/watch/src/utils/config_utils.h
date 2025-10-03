#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

#include <stdbool.h>
#include <rtdevice.h>
#include "flashdb/fdb.h"
#include "utils/datetime_utils.h"

void load_configurations(rt_device_t lcd);

#endif // CONFIG_UTILS_H