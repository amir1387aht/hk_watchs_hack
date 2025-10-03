/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CUSTOM_MEM_MAP__
#define __CUSTOM_MEM_MAP__

#undef  BLE_ROM_BASE
#define BLE_ROM_BASE        (0x100000)

#undef  BLE_RAM_BASE
#define BLE_RAM_BASE        (0x00115000)

#undef  BLE_RAM_SIZE
#define BLE_RAM_SIZE        (44*1024)

#undef  BCPU_RAM_DATA_SIZE
#define BCPU_RAM_DATA_SIZE  (42*1024+512)

#endif  /* __MEM_MAP__ */

