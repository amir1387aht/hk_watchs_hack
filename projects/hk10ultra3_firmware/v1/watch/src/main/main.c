#include <rtthread.h>
#include "drv_flash.h"
#include "drv_io.h"
#include <bf0_hal.h>
#include <board.h>
#include <rtdevice.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef RT_USING_MODULE
    #include "dlfcn.h"
    #include "dlmodule.h"
#endif

#ifdef BSP_USING_DFU
    #include "dfu.h"
#endif

#ifdef RWBT_ENABLE
    #include "rwip.h"
    #if (NVDS_SUPPORT)
        #include "sifli_nvds.h"
    #endif
#endif

#ifdef RT_USING_XIP_MODULE
    #include "dfs_posix.h"
#endif

#ifdef BSP_BLE_TIMEC
    #include "bf0_ble_tipc.h"
#endif

#include "flashdb/fdb.h"

#ifdef RT_USING_DFS_MNTTABLE
    #include "dfs_fs.h"
#endif

#define BOOT_LOCATION 1

int main(void)
{
#ifdef SOC_BF0_LCPU
    env_init();

    #if (NVDS_SUPPORT)
        #ifdef RT_USING_PM
            #ifdef PM_STANDBY_ENABLE
    g_boot_mode = rt_application_get_power_on_mode();
    if (g_boot_mode == 0)
            #endif
        #endif
    {
        sifli_nvds_init();
    }
    #else
    rwip_init(0);
    #endif
#endif

#ifdef BSP_BLE_TIMEC
    ble_tipc_init(true);
#endif

    return RT_EOK;
}

#ifdef RT_USING_MODULE

int mod_load(int argc, char **argv)
{
    if (argc < 2)
        return -1;
    dlopen(argv[1], 0);
    return 0;
}
MSH_CMD_EXPORT(mod_load, Load module);

    #ifdef RT_USING_XIP_MODULE

static struct rt_dlmodule *test_module;

int mod_run(int argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("wrong argument\n");
        return -1;
    }

    const char *mod_name = argv[1];
    const char *install_path = (argc >= 3) ? argv[2] : "/apps";

    test_module = dlrun(mod_name, install_path);

    if (test_module->nref && test_module->init_func)
        test_module->init_func(test_module);

    return 0;
}
MSH_CMD_EXPORT(mod_run, Run module);

int get_f_phy_addr(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    int fid = open(argv[1], O_RDONLY);
    if (fid >= 0)
    {
        uint32_t addr;
        uint8_t buf[10];
        read(fid, buf, 5);

        if (ioctl(fid, F_GET_PHY_ADDR, &addr) >= 0)
            rt_kprintf("addr: 0x%p\n", addr);

        close(fid);
    }

    return 0;
}
MSH_CMD_EXPORT(get_f_phy_addr, Get file physical address);

    #endif // RT_USING_XIP_MODULE

int mod_free(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    struct rt_dlmodule *hdl = dlmodule_find(argv[1]);
    if (hdl)
        dlclose((void *)hdl);

    return 0;
}
MSH_CMD_EXPORT(mod_free, Free module);

#endif // RT_USING_MODULE