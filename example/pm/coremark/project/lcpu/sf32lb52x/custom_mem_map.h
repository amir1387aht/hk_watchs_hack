/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CUSTOM_MEM_MAP__
#define __CUSTOM_MEM_MAP__

#undef LCPU_RAM_DATA_START_ADDR
#define LCPU_RAM_DATA_START_ADDR  LPSYS_EM_BASE

#undef LCPU_RAM_DATA_SIZE
#define LCPU_RAM_DATA_SIZE        LPSYS_EM_SIZE

/* should be aligned with HCPU custom_mem_map */
#undef LPSYS_RAM_SIZE
#define LPSYS_RAM_SIZE (32*1024)

#endif  /* __MEM_MAP__ */

