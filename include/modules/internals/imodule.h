/**
 * @file modules/internals/imodule.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include "general/sdk_status.h"
#include "modules/internals/abi.h"
#include "modules/internals/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A pseudo-base "class" for a module. Stores unique internal data for that module.
 */
typedef struct IModule IModule;

// ================================== UTILS ==================================

/**
 * @brief Allocates memory, initializes `IModule` and returns pointer to that
 * @param name Name of module (non-empty and not NULL string)
 * @param description Description of module (non-empty and not NULL string)
 * @param server_functions Structure with pointers to server functions (for example, for logging)
 * @param poll_ratio Poll ratio of module
 * @param is_enabled Is module enabled? (`1` if enabled, otherwise `0`)
 * @return Pointer to initialized `IModule` if success or `NULL` if error
 */
SDK_EXPORT IModule *sdk_imodule_create(const char               *name,
                                       const char               *description,
                                       ABI_SERVER_CORE_FUNCTIONS server_functions,
                                       uint32_t                  poll_ratio,
                                       uint8_t                   is_enabled);

/**
 * @brief Destroys module and deallocates memory allocated via `sdk_imodule_create`
 * @param module Not-null pointer to `IModule`. If a null pointer is passed, there will be no
 * effect.
 */
SDK_EXPORT void sdk_imodule_destroy(IModule *module);

/**
 * @brief Get context of the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 * @return Pointer to `ABI_MODULE_CONTEXT`
 */
SDK_EXPORT const ABI_MODULE_CONTEXT *sdk_imodule_get_context(const IModule *module);

/**
 * @brief Set context of the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 * @param context A context whose fields are allocated via `malloc`
 * @note The function frees the memory allocated for the old context, so you don't need to worry
 * about it.
 */
SDK_EXPORT void sdk_imodule_set_context(IModule *module, ABI_MODULE_CONTEXT context);

/**
 * @brief Get MDTP data of the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 * @return Pointer to `ABI_MODULE_MDTP_DATA`
 */
SDK_EXPORT const ABI_MODULE_MDTP_DATA *sdk_imodule_get_mdtp_data(const IModule *module);

/**
 * @brief Set MDTP data of the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 * @param data MDTP data whose `data` field is allocated via `malloc`
 * @note The function frees the memory allocated for the old data, so you don't need to worry
 * about it.
 */
SDK_EXPORT void sdk_imodule_set_mdtp_data(IModule *module, ABI_MODULE_MDTP_DATA data);

/**
 * @brief Get poll ratio of the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 * @return `uint32_t` with poll ratio
 */
SDK_EXPORT uint32_t sdk_imodule_get_poll_ratio(const IModule *module);

/**
 * @brief Sets poll ratio of the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 * @param poll_ratio Poll ratio
 */
SDK_EXPORT void sdk_imodule_set_poll_ratio(IModule *module, uint32_t poll_ratio);

/**
 * @brief Enables the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 */
SDK_EXPORT void sdk_imodule_enable(IModule *module);

/**
 * @brief Disables the module using pointer to `IModule`
 * @param module Not-null pointer to `IModule`
 */
SDK_EXPORT void sdk_imodule_disable(IModule *module);

/**
 * @brief Check if the module is enabled
 * @param module Not-null pointer to `IModule`
 * @return `1` if the module is enabled, otherwise `0`
 */
SDK_EXPORT uint8_t sdk_imodule_is_enabled(const IModule *module);

/**
 * @brief Get structure with pointer to module functions
 * @param module Not-null pointer to `IModule`
 * @return Pointer to `ABI_MODULE_FUNCTIONS`
 */
SDK_EXPORT const ABI_MODULE_FUNCTIONS *sdk_imodule_get_module_functions(const IModule *module);

/**
 * @brief Get structure with pointer to server core functions
 * @param module Not-null pointer to `IModule`
 * @return Pointer to `ABI_SERVER_CORE_FUNCTIONS`
 */
SDK_EXPORT const ABI_SERVER_CORE_FUNCTIONS *sdk_imodule_get_server_core_functions(
    const IModule *module);


// ================================== REGISTERERS ==================================

/**
 * @brief Registers a module destroy function that will be called by the server
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Pointer to destroy function with signature `void (void)`
 * @note Call sdk_imodule_destroy at the end of the implementation of this function to destroy the
 * `IModule`
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_destroy(IModule *module, void (*callback)(void));

/**
 * @brief Registers a module function that returns JSON configuration and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `const char *(void)`. If `NULL`, no
 * effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_get_configuration(IModule *module,
                                                      const char *(*callback)(void));

/**
 * @brief Registers a module function that returns MDTP data and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `const ABI_MODULE_MDTP_DATA *(void)`.
 * If `NULL`, no effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_get_data(IModule *module,
                                             const ABI_MODULE_MDTP_DATA *(*callback)(void));

/**
 * @brief Registers a module function that enables the module and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `void(void)`. If `NULL`, no effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_enable(IModule *module, void (*callback)(void));

/**
 * @brief Registers a module function that disables the module and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `void(void)`. If `NULL`, no effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_disable(IModule *module, void (*callback)(void));

/**
 * @brief Registers a module function that checks if the module is enabled and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `uint8_t(void)`. If `NULL`, no
 * effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_is_enabled(IModule *module, uint8_t (*callback)(void));

/**
 * @brief Registers a module function that returns the module name and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `const char *(void)`. If `NULL`, no
 * effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_get_module_name(IModule *module, const char *(*callback)(void));

/**
 * @brief Registers a module function that returns the module description and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `const char *(void)`. If NULL, no
 * effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_get_module_description(IModule *module,
                                                           const char *(*callback)(void));

/**
 * @brief Registers a module function that sets the poll ratio and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.
 * @param callback Not-null pointer to function with signature `void(uint32_t)`. If `NULL`, no
 * effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_set_poll_ratio(IModule *module,
                                                   void (*callback)(uint32_t poll_ratio));

/**
 * @brief Registers a module function that returns the poll ratio and will be called by the
 * server core
 * @param module Not-null pointer to `IModule`. If `NULL`, no effect.`
 * @param callback Not-null pointer to function with signature `uint32_t(void)`. If `NULL`, no
 * effect.
 * @note Do not block thread in this function
 */
SDK_EXPORT void sdk_module_register_get_poll_ratio(IModule *module, uint32_t (*callback)(void));


#ifdef __cplusplus
}
#endif
