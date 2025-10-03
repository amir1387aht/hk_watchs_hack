/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     zylx         first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <bf0_hal.h>
#include <board.h>
#include <string.h>
#include <stdbool.h>
#include "lvgl.h"
#ifdef RT_USING_MODULE
    #include "dlmodule.h"
    #include "dlfcn.h"
    #include "mod_installer.h"
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
#endif /* RT_USING_MODULE */

#ifdef BSP_BLE_TIMEC
    #include "bf0_ble_tipc.h"
#endif


#ifdef RT_USING_DFS_MNTTABLE
#include "dfs_fs.h"
const struct dfs_mount_tbl mount_table[] =
{
    {
        .device_name = "flash2",
        .path = "/",
        .filesystemtype = "elm",
        .rwflag = 0,
        .data = 0,
    },
    {
        0,
    },
};
#endif
#ifdef RT_USING_DFS
#include "dfs_file.h"
#include "dfs_posix.h"
#ifndef BSP_USING_PC_SIMULATOR
    #include "drv_flash.h"
#endif /* !BSP_USING_PC_SIMULATOR */

#ifndef FS_REGION_START_ADDR
    #error "Need to define file system start address!"
#endif

#define FS_ROOT "root"

/**
 * @brief Mount fs.
 */
int mnt_init(void)
{
    register_mtd_device(FS_REGION_START_ADDR, FS_REGION_SIZE, FS_ROOT);
    if (dfs_mount(FS_ROOT, "/", "elm", 0, 0) == 0) // fs exist
    {
        rt_kprintf("mount fs on flash to root success\n");
    }
    else
    {
        // auto mkfs, remove it if you want to mkfs manual
        rt_kprintf("mount fs on flash to root fail\n");
        if (dfs_mkfs("elm", FS_ROOT) == 0)//Format file system
        {
            rt_kprintf("make elm fs on flash sucess, mount again\n");
            if (dfs_mount(FS_ROOT, "/", "elm", 0, 0) == 0)
                rt_kprintf("mount fs on flash success\n");
            else
                rt_kprintf("mount to fs on flash fail\n");
        }
        else
            rt_kprintf("dfs_mkfs elm flash fail\n");
    }
    return RT_EOK;
}
INIT_ENV_EXPORT(mnt_init);
#endif /* RT_USING_DFS */


// #define BOOT_LOCATION 1
int main(void)
{
    int count = 0;

#ifdef SOC_BF0_LCPU
    env_init();
    //sifli_mbox_init();
#if (NVDS_SUPPORT)
#ifdef RT_USING_PM
#ifdef PM_STANDBY_ENABLE
    g_boot_mode = rt_application_get_power_on_mode();
    if (g_boot_mode == 0)
#endif // PM_STANDBY_ENABLE
#endif // RT_USING_PM
    {
        sifli_nvds_init();
    }
#else
    rwip_init(0);
#endif // NVDS_SUPPORT

#endif

#ifdef BSP_BLE_TIMEC
    ble_tipc_init(true);
#endif

    return RT_EOK;
}

#ifdef RT_USING_MODULE
extern void gui_app_fwk_test_start(void);
extern void gui_app_fwk_test_stop(void);
int mod_load(int argc, char **argv)
{
    rt_kprintf("%s argc=%d\n", __func__, argc);
    if (argc < 2)
        return -1;
    gui_app_fwk_test_start();
    dlopen(argv[1], 0);
    gui_app_fwk_test_stop();
    return 0;
}
MSH_CMD_EXPORT(mod_load, Load module);

int mod_free(int argc, char **argv)
{
    struct rt_dlmodule *hdl;
    if (argc < 2)
        return -1;

    hdl = dlmodule_find(argv[1]);
    if (hdl != RT_NULL)
        dlclose((void *)hdl);
    return 0;
}
MSH_CMD_EXPORT(mod_free, Free module);

int mod_open(int argc, char **argv)
{
    rt_kprintf("%s argc=%d\n", __func__, argc);
    if (argc < 3)
        return -1;
    gui_app_fwk_test_start();
    app_open(argv[1], argv[2]);
    gui_app_fwk_test_stop();
    return 0;
}
MSH_CMD_EXPORT(mod_open, Open module);

int mod_close(int argc, char **argv)
{
    struct rt_dlmodule *hdl;
    if (argc < 2)
        return -1;

    hdl = dlmodule_find(argv[1]);
    if (hdl != RT_NULL)
        app_close((void *)hdl);
    else
        rt_kprintf("can't not fine module:%s\n", argv[1]);
    return 0;
}
MSH_CMD_EXPORT(mod_close, Close module);

#ifdef RT_USING_XIP_MODULE

static struct rt_dlmodule *test_module;
int mod_run(int argc, char **argv)
{
    const char *install_path;
    const char *mod_name;

    if (argc < 2)
    {
        rt_kprintf("wrong argument\n");
        return -1;
    }

    mod_name = argv[1];
    if (argc >= 3)
    {
        install_path = argv[2];
    }
    else
    {
        install_path = "/app";
    }

    test_module = dlrun(mod_name, install_path);
    rt_kprintf("run %s:0x%x,%d\n", mod_name, test_module, test_module->nref);
    if (test_module->nref)
    {
        if (test_module->init_func)
        {
            test_module->init_func(test_module);
        }
    }
    return 0;
}
MSH_CMD_EXPORT(mod_run, Run module);
#endif /* RT_USING_XIP_MODULE */

#endif


/**
 * Export API for dlmodule
 */
RTM_EXPORT(lv_label_set_text);
RTM_EXPORT(lv_disp_get_scr_act);
RTM_EXPORT(lv_disp_get_default);
RTM_EXPORT(lv_obj_set_size);
RTM_EXPORT(lv_label_create);
RTM_EXPORT(lv_obj_align);
RTM_EXPORT(lv_obj_align_to);
RTM_EXPORT(lv_obj_set_align);
RTM_EXPORT(lv_obj_set_style_text_font);
RTM_EXPORT(lv_img_set_src);
RTM_EXPORT(lv_img_create);
#ifdef LV_USING_EXT_RESOURCE_MANAGER
    #include "lv_ext_resource_manager.h"
    RTM_EXPORT(lv_ext_res_mng_builtin_init);
    RTM_EXPORT(lv_ext_res_mng_builtin_destroy);
    RTM_EXPORT(lv_ext_get_locale);
    RTM_EXPORT(lv_ext_set_locale);
    RTM_EXPORT(lv_ext_str_get);
    RTM_EXPORT(lv_ext_get_lang_pack_list);
#endif /* LV_USING_EXT_RESOURCE_MANAGER */

#include "lvsf_font.h"
// #include "lv_font_fmt_txt.h"
RTM_EXPORT(lv_ext_set_local_font_bitmap);
RTM_EXPORT(lv_font_get_glyph_dsc_fmt_txt);
RTM_EXPORT(lv_font_get_bitmap_fmt_txt);
#include "lv_font.h"
RTM_EXPORT(lv_font_montserrat_28);

