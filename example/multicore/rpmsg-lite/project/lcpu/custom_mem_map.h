/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CUSTOM_MEM_MAP__
#define __CUSTOM_MEM_MAP__

#ifdef USING_PARTITION_TABLE
    #include "ptab.h"
#endif /* USING_PARTITION_TABLE */

#define HL_IPC_TX_BUF_SIZE         (256)
#define RPMSG_BUF_SIZE             (4096)

#undef HCPU_RAM_DATA_SIZE
#define HCPU_RAM_DATA_SIZE (HPSYS_RAM_SIZE - HCPU_RO_DATA_SIZE - HPSYS_MBOX_BUF_SIZE - HL_IPC_TX_BUF_SIZE - HPSYS_RETM_SIZE)

#undef LCPU_RAM_DATA_SIZE
#define LCPU_RAM_DATA_SIZE (LPSYS_RAM_SIZE - LCPU_MBOX_SIZE - LCPU_RAM_CODE_SIZE - RPMSG_BUF_SIZE)

#endif  /* __CUSTOM_MEM_MAP__ */

