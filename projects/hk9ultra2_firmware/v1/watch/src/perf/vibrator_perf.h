#ifndef VIBRATOR_GPIO_H
#define VIBRATOR_GPIO_H

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include "bf0_hal.h"
#include "bf0_hal_pinmux.h"
#include "drv_io.h"
#include "board.h"

/* Initialize the vibrator GPIO (PAD_PA20 as GPIO, default LOW). */
void vibrator_init(void);

/* Turn vibrator on (GPIO HIGH). */
void vibrator_on(void);

/* Turn vibrator off (GPIO LOW). */
void vibrator_off(void);

/* Buzz for `ms` milliseconds (blocking). */
void vibrator_buzz(uint16_t ms);

/* simple on/off burst pattern (even indexes = ON ms, odd = OFF ms).
 */
void vibrator_burst_pattern(const uint16_t *bursts, size_t count);

#endif /* VIBRATOR_GPIO_H */