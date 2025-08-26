/**
 * @file modules/internals/imodule.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include "general/sdk_status.h"
#include "modules/internals/abi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A pseudo-base "class" for a module. Stores unique internal data for that module.
 */
typedef struct IModule {
    ABI_MODULE_CONTEXT   context;    ///< Context of the module
    ABI_MODULE_MDTP_DATA mdtp_data;  ///< Temporary buffer for MDTP data
    uint32_t             poll_ratio; ///< Poll ratio of the module
    uint8_t              is_enabled; ///< `1` if module is enabled, otherwise `0`
} IModule;

/**
 * @brief Virtual functions table for `IModule`
 * @note You need to fill this table yourself
 */
typedef struct ModuleVTable {
    /**
     * @brief A user function called for final module initialization. You can do whatever you want
     * in it
     * @param module Pointer to `IModule`
     * @param json Json configuration
     * @return `SDKStatus` with error code. You should return SDK_OK if no errors occured
     * @note Do not block thread in this function
     */
    SDKStatus (*init)(IModule *module, const char *json);

    /**
     * @brief A user function called when module destroys. You can do whatever you want
     * in it
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     */
    void (*destroy)(IModule *module);

    /**
     * @brief A user function called when there is need to obtain module configuration
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     */
    const char *(*get_configuration)(IModule *module);

    /**
     * @brief A user function called when there is need to obtain module data.
     *
     * Use `sdk_mdtp_make_value`, `sdk_mdtp_make_container` to make nodes and `sdk_mdtp_make_root`
     * to finalize result (create root frame and return it)
     *
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     */
    ABI_MODULE_MDTP_DATA *(*get_data)(IModule *m);

    /**
     * @brief A user function called when there is need to enable the module
     *
     * By default, you should set `module->is_enabled` to `1`
     *
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     */
    void (*enable)(IModule *module);

    /**
     * @brief A user function called when there is need to disable the module
     *
     * By default, you should set `module->is_enabled` to `0`
     *
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     */
    void (*disable)(IModule *module);

    /**
     * @brief A user function called when there is need to check if the module is enabled
     *
     * By default, you should return `module->is_enabled`
     *
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     * @return `1` if the module is enabled, otherwise `0`
     */
    uint8_t (*is_enabled)(IModule *module);

    /**
     * @brief A user function called when there is need to get module name
     *
     * By default, you should return `module->context.module_name`
     *
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     * @return Name of the module
     */
    const char *(*get_name)(IModule *module);

    /**
     * @brief A user function called when there is need to get module description
     *
     * By default, you should return `module->context.module_description`
     *
     * @param module Pointer to `IModule`
     * @note Do not block thread in this function
     * @return Description of the module
     */
    const char *(*get_desc)(IModule *module);

    /**
     * @brief A user function called when there is need to set poll ratio of the module
     *
     * By default, you should set `module->poll_ratio` to `ratio`
     *
     * @param module Pointer to `IModule`
     * @param ratio Required poll ratio
     * @note Do not block thread in this function
     */
    void (*set_poll_ratio)(IModule *module, uint32_t ratio);

    /**
     * @brief A user function called when there is need to set poll ratio of the module
     *
     * By default, you should return `module->poll_ratio`
     *
     * @param module Pointer to `IModule`
     * @param ratio Required poll ratio
     * @note Do not block thread in this function
     * @return Poll ratio of the module
     */
    uint32_t (*get_poll_ratio)(IModule *module);
} ModuleVTable;

#ifdef __cplusplus
}
#endif
