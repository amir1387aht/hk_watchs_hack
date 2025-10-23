#ifndef CROWN_PREF_H
#define CROWN_PREF_H

#include "rtconfig.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "rtdbg.h"
#include "board.h"

// Initialize GPIO32 and GPIO33 with IRQ
int crown_init(void);

// Register callbacks for pins
typedef void (*gpio_callback_t)();

// Callbacks
void crown_scroll_up_cb(gpio_callback_t cb);
void crown_scroll_down_cb(gpio_callback_t cb);

#endif // CROWN_PREF_H