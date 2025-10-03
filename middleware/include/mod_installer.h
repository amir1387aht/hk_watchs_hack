/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MOD_INSTALLER_H
#define MOD_INSTALLER_H
#include "rtthread.h"
#include <stdint.h>
#include <stdbool.h>
#ifdef BUILD_DLMODULE
    #include "dlmodule.h"
#endif /* RT_USING_MODULE */

/**
 ****************************************************************************************
* @addtogroup mod_installer Module Installer
* @ingroup middleware
* @brief Install module to run in flash
* @{
****************************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

#define DLMODULE_STRINGIFY_(val)       #val

/** Converts a macro argument into a character constant.
 */
#define DLMODULE_STRINGIFY(val)       DLMODULE_STRINGIFY_(val)

#define DLMODULE_CONCAT_2_(p1, p2)     p1##p2

/**@brief Concatenates two parameters.
 *
 * It realizes two level expansion to make it sure that all the parameters
 * are actually expanded before gluing them together.
 *
 * @param p1 First parameter to concatenating
 * @param p2 Second parameter to concatenating
 *
 * @return Two parameters glued together.
 *         They have to create correct C mnemonic in other case
 *         preprocessor error would be generated.
 *
 */
#define DLMODULE_CONCAT_2(p1, p2)      CONCAT_2_(p1, p2)

typedef void (*module_init_func_t)(void);
typedef void (*module_cleanup_func_t)(void);

#ifdef BUILD_DLMODULE

#if 0
#define MODULE_DEF(name, data_type)              \
    typedef struct                               \
    {                                            \
        void *res_mng;                           \
        data_type custom_data;                   \
    } DLMODULE_CONCAT_2(name,data_type)
#endif

#define MODULE_CONTEXT          ((MODULE_USER_DATA_TYPE *)(dlmodule_get_user_data(DLMODULE_STRINGIFY(MODULE_NAME), sizeof(MODULE_USER_DATA_TYPE))))

#define MODULE_CONTEXT_DEFINE

#define MODULE_CONTEXT_ALLOC()  MODULE_CONTEXT

#define MODULE_CONTEXT_FREE()   dlmodule_free_user_data(DLMODULE_STRINGIFY(MODULE_NAME))

#define MODULE_GET()              dlmodule_find(DLMODULE_STRINGIFY(MODULE_NAME))

#ifdef LV_USING_EXT_RESOURCE_MANAGER

#include "lv_ext_resource_manager_builtin.h"

#define MODULE_INIT_DEF(init_func)               \
    void module_init(struct rt_dlmodule *module) \
    {                                            \
        module_init_func_t func = init_func;     \
        void *res_mng = rt_malloc(sizeof(struct lv_ext_res_mng_tag));  \
        dlmodule_set_res_mng(module, res_mng);                         \
        lv_ext_res_mng_builtin_init(res_mng, lv_i18n_lang_pack);       \
        lv_ext_set_locale(res_mng, lv_ext_get_locale());   \
        if (func)                                \
        {                                        \
            func();                              \
        }                                        \
    }

#define MODULE_CLEANUP_DEF(cleanup_func)            \
    void module_cleanup(struct rt_dlmodule *module) \
    {                                               \
        module_cleanup_func_t func = cleanup_func;  \
        void *res_mng = dlmodule_get_res_mng(module); \
        if (res_mng)                                \
        {                                           \
            lv_ext_res_mng_builtin_destroy(res_mng);\
            rt_free(res_mng);                       \
        }                                           \
        if (func)                                   \
        {                                           \
            func();                                 \
        }                                           \
    }
#else
#define MODULE_INIT_DEF(init_func)               \
        void module_init(struct rt_dlmodule *module) \
        {                                            \
            module_init_func_t func = init_func;     \
            if (func)                                \
            {                                        \
                func();                              \
            }                                        \
        }

#define MODULE_CLEANUP_DEF(cleanup_func)            \
    void module_cleanup(struct rt_dlmodule *module) \
    {                                               \
        module_cleanup_func_t func = cleanup_func;  \
        if (func)                                   \
        {                                           \
            func();                                 \
        }                                           \
    }
#endif

#include "module_name.h"

#else

#define MODULE_CONTEXT         (DLMODULE_CONCAT_2(MODULE_NAME, _ctx))

#define MODULE_CONTEXT_DEFINE  static MODULE_USER_DATA_TYPE *MODULE_CONTEXT

#define MODULE_CONTEXT_ALLOC() do { MODULE_CONTEXT = rt_malloc(sizeof(MODULE_USER_DATA_TYPE)); RT_ASSERT(MODULE_CONTEXT)} while (0)

#define MODULE_CONTEXT_FREE()  do {rt_free(MODULE_CONTEXT); MODULE_CONTEXT = RT_NULL;} while (0)

#define MODULE_INIT_DEF(init_func)

#define MODULE_CLEANUP_DEF(cleanup_func)

#define MODULE_NAME           module

#endif /* BUILD_DLMODULE */

/** Application installer info */
typedef struct
{
    const char *app_name;             /**< App name  */
    const char *res_package_path;     /**< App resource .so file full path */
    const char *res_install_path;     /**< where to install resource module  */
    const char *pgm_package_path;     /**< App program .so file full path */
    const char *pgm_install_path;     /**< where to install program module  */
} app_installer_info_t;

/**
 * @brief  Install dlmodule
 *
 *
 * @param[in]  mod_name module name
 * @param[in]  package_path .so file path
 * @param[in]  install_path where to install the module
 *
 * @retval RT_EOK: success, other: fail, negative value for error code, such as -RT_ERROR
 */
rt_err_t mod_install(const char *mod_name, const char *package_path, const char *install_path);

/**
 * @brief  Install application
 *
 *
 * @param[in]  info Application installer info
 *
 * @retval RT_EOK: success, other: fail, negative value for error code, such as -RT_ERROR
 */
rt_err_t app_install(app_installer_info_t *info);

/**
 * @brief  Uninstall application
 *
 *
 * @param[in]  app_name application name
 * @param[in]  res_install_path Resource install path
 * @param[in]  pgm_install_path Program install path
 *
 * @retval 0: success, other: fail
 */
int32_t app_uninstall(const char *app_name, const char *res_install_path, const char *pgm_install_path);

/**
 * @brief Init mod_installer
 *
 *
 * @param[in]  sys_prog_start_addr Start address of system program
 * @param[in]  sys_prog_end_addr End address of system program
 *
 * @retval 0: success, other: fail
 */
int32_t mod_installer_init(uint32_t sys_prog_start_addr, uint32_t sys_prog_end_addr);

/**
 * @brief Open application
 *
 *
 * @param[in]  res_package_path App resource .so file full path
 * @param[in]  package_path App program .so file full path
 *
 * @retval module instance of App
 */
struct rt_dlmodule *app_open(const char *res_package_path, const char *package_path);

/**
 * @brief Close application
 *
 *
 * @param[in]  module Module instance of App
 *
 * @retval RT_EOK: success, other: fail
 */
rt_err_t app_close(struct rt_dlmodule *module);

/// @}  mod_installer

#ifdef __cplusplus
}
#endif

/// @} file
#endif

