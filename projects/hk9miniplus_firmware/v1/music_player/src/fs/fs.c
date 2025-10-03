#include "./fs.h"

const struct dfs_mount_tbl mount_table[] = {
{
     .device_name = REGION_NAME,
     .path = FS_PATH,
     .filesystemtype = FS_NAME,
     .rwflag = 0,
     .data = 0
},
    {0}
};

int mnt_init(void)
{
    register_mtd_device(FS_REGION_START_ADDR, FS_REGION_SIZE, FS_ROOT);

    if (!dfs_mount(FS_ROOT, FS_PATH, FS_NAME, 0, 0))
        rt_kprintf("mount fs on flash to root success\n");
    else
    {
        rt_kprintf("mount fs on flash to root fail\n");
        if (!dfs_mkfs(FS_NAME, FS_ROOT))
        {
            rt_kprintf("make fs on flash success, mount again\n");
            rt_kprintf(dfs_mount(FS_ROOT, FS_PATH, FS_NAME, 0, 0)
                ? "mount fs on flash success\n"
                : "mount to fs on flash fail\n");
        }
        else rt_kprintf("dfs_mkfs flash fail\n");
    }

    if (flashdb_init() != RT_EOK)
        return RT_ERROR;

    return RT_EOK;
}

INIT_ENV_EXPORT(mnt_init);