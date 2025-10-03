/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

extern uint32_t central_gatt_write(uint32_t count);

int main(void)
{
    printk("Finish gatt write %d\n", central_gatt_write(0U));
    return 0;
}
