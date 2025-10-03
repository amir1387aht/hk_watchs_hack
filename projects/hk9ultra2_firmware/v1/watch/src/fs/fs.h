#ifndef FS_UTILS_H
#define FS_UTILS_H

#include "dfs_file.h"
#include "dfs_fs.h"
#include "dfs_posix.h"
#include "drv_flash.h"
#include <custom_mem_map.h>
#include <rtthread.h>
#include "../flashdb/fdb.h"

#define FS_ROOT "root"
#define FS_NAME "yaffs"
#define REGION_NAME "flash2"
#define FS_PATH "/"

int mnt_init(void);

#endif // FS_UTILS_H