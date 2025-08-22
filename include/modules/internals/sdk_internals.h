/**
 * @file internals/sdk_internals.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include "modules/internals/abi.h"

#if defined _WIN32
#define SDK_ABI __declspec(dllexport)
#else
#define SDK_ABI __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif


// =========================== ABI FUNCTIONS ===========================

/**
 * @internal
 * @brief Initialize the module. Entry function
 * @param server_functions Struct with server functions
 * @param json_configuration Json configuration
 * @return Pointer to `ABI_MODULE_FUNCTIONS` with module functions
 * @endinternal
 */
SDK_ABI ABI_MODULE_FUNCTIONS *module_init(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                          const char               *json_configuration);

/**
 * @internal
 * @brief Calls `sdk_module_destroy` and frees resources
 * @endinternal
 */
SDK_ABI void module_destroy(void);


// =========================== SDK UTILITY FUNCTIONS ===========================


typedef enum SDKStatus SDKStatus; // Forward declaration
typedef enum LogType   LogType;   // Forward declaration

/**
 * @brief Logs. Logs are displayed on the server.
 * @param log_type Log type. See `LogType`
 * @param message The message that will be displayed
 * @return `SDKStatus` which describes the return status of the function (success or error)
 */
SDKStatus sdk_utils_log(LogType log_type, const char *message);

/**
 * @brief Configures the module (writes the module name and description to the internal structure)
 * @param module_name Module name
 * @param module_description Module description (purpose of the module)
 * @return `SDKStatus` which describes the return status of the function (success or error)
 */
SDKStatus sdk_utils_module_setup(const char *module_name, const char *module_description);

/**
 * @brief Get module name using context
 * @return Module name
 */
const char *sdk_utils_get_module_name(void);

/**
 * @brief Get module name using context
 * @return Module name
 */
const char *sdk_utils_get_module_description(void);

/**
 * @brief Get module name using context
 * @return Module context
 */
ABI_MODULE_CONTEXT *sdk_utils_get_module_context(void);


#ifdef __cplusplus
}
#endif
