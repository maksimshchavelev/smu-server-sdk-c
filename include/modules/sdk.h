/**
 * @file sdk.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include "modules/internals/mdtp.h"
#include <stdint.h>

#if defined _WIN32
#define SDK_ABI __declspec(dllexport)
#else
#define SDK_ABI __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ABI_MODULE_MDTP_DATA      ABI_MODULE_MDTP_DATA;      ///< Forward declaration
typedef struct ABI_MODULE_CONTEXT        ABI_MODULE_CONTEXT;        ///< Forward declaration
typedef struct ABI_MODULE_FUNCTIONS      ABI_MODULE_FUNCTIONS;      ///< Forward declaration
typedef struct ABI_SERVER_CORE_FUNCTIONS ABI_SERVER_CORE_FUNCTIONS; ///< Forward declaration

/**
 * @brief Describes the return value of some SDK utility functions
 */
typedef enum SDKStatus {
    SDK_OK,                        ///< Success
    SDK_INVALID_ARGUMENT,          ///< Invalid argument(s) (for example, NULL pointer)
    SDK_ARGUMENT_PROCESSING_ERROR, ///< Use for example when json parsing failed
    SDK_ALLOCATION_ERROR,          ///< Used, for example, when `malloc` or `strdup` fails.
    SDK_INTERNALS_UNINITIALIZED,   ///< Used when internal structures are not initialized. For
                                 ///< example, you use `sdk_log` but did not call `sdk_module_setup`
                                 ///< in the `sdk_module_init` function.
    SDK_OTHER_ERROR ///< Used for other errors when nothing fits
} SDKStatus;

/**
 * @brief Type of logs
 * @see sdk_log
 */
typedef enum LogType {
    LOG_INFO,    ///< **White** logs
    LOG_WARNING, ///< **Yellow** logs
    LOG_ERROR    ///< **Red** logs
} LogType;


/**
 * @brief Structure with SDK utility functions, such as logging or Json parsing. See details for
 * usage example
 *
 * @section example_usage Example usage
 * @code{.c}
 * sdk_utils.log(LOG_INFO, "Hello from my module!");
 * @endcode
 */
typedef struct SDK_UTILS {
    /**
     * @brief Logging function
     * @param log_type Log type. See `LogType`
     * @param message Message for logging
     */
    SDKStatus (*log)(LogType log_type, const char *message);

    /**
     * @brief Configures the module (writes the module name and description to the internal
     * structure)
     * @note Be sure to call this function in `sdk_module_init`
     * @param module_name Module name
     * @param module_description Module description (purpose of the module)
     * @return `SDKStatus` which describes the return status of the function (success or error)
     */
    SDKStatus (*module_setup)(const char *module_name, const char *module_description);

    /**
     * @brief Get module name using context
     * @return Module name
     */
    const char *(*get_module_name)(void);

    /**
     * @brief Get module name using context
     * @return Module name
     */
    const char *(*get_module_description)(void);

    /**
     * @brief Get module name using context
     * @return Module context
     */
    ABI_MODULE_CONTEXT *(*get_module_context)(void);

    MDTP_UTILS mdtp; ///< MDTP utils

} SDK_UTILS;

extern SDK_UTILS sdk_utils; ///< Implementation are in sdk.c

/**
 * @brief Called when the module is initialized
 * @param json_config Json module configuration
 * @note Implement this function in your module yourself
 * @warning Be sure to call `sdk_utils.module_setup` to set the module name and description
 * @return `SDKStatus` which describes the return status of the function (success or error)
 * @see SDK_UTILS
 */
SDK_ABI SDKStatus sdk_module_init(const char *json_config);

/**
 * @brief Called when the module is destroyed.
 * @note Implement this function in your module yourself
 */
SDK_ABI void sdk_module_destroy(void);

/**
 * @brief Called when a module's configuration needs to be retrieved.
 * @note Implement this function in your module yourself
 * @return Raw string with module configuration
 */
SDK_ABI const char *sdk_module_get_configuration(void);

/**
 * @brief Get data of module
 * @note Implement this function in your module yourself
 * @return MDTP encoded data with metrics
 */
SDK_ABI ABI_MODULE_MDTP_DATA *sdk_module_get_data(void);

/**
 * @brief Called when the module is activated.
 * @note Implement this function in your module yourself
 */
SDK_ABI void sdk_module_enable(void);

/**
 * @brief Called when the module is deactivated.
 * @note Implement this function in your module yourself
 */
SDK_ABI void sdk_module_disable(void);

/**
 * @brief Called when you need to know if a module is active
 * @note Implement this function in your module yourself
 * @return Should return `1` if the module is active, otherwise `0`
 */
SDK_ABI uint8_t sdk_module_is_enabled(void);


#ifdef __cplusplus
}
#endif
