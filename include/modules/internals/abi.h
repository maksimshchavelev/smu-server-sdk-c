/**
 * @file modules/internals/mdtp.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Struct to storing MDTP data. See documentation for MDTP protocol.
 * @note This is a **packaged** structure.
 */
typedef struct __attribute__((packed)) ABI_MODULE_MDTP_DATA {
    const void *data; ///< Pointer to MDTP bytes
    uint32_t    size; ///< Count of MDTP bytes (size of data)
} ABI_MODULE_MDTP_DATA;


/**
 * @brief Context of module
 *
 * The context is needed so that the server core knows what module is accessing it (when calling
 * core functions, for example, logging)
 *
 * @note This is a **packaged** structure
 */
typedef struct __attribute__((packed)) ABI_MODULE_CONTEXT {
    const char *module_name;        ///< Name of module
    const char *module_description; ///< Description of module
} ABI_MODULE_CONTEXT;


/**
 * @internal
 * @brief Functions provided by the server for the module
 *
 * This structure will be passed to the module initialization function.
 *
 * @note This is a packaged structure.
 * @endinternal
 */
typedef struct __attribute__((packed)) ABI_SERVER_CORE_FUNCTIONS {
    int (*abi_get_abi_version)(ABI_MODULE_CONTEXT *context); ///< Returns ABI version

    void (*abi_log)(ABI_MODULE_CONTEXT *context,
                    int                 log_type,
                    const char         *message); ///< Logging function. Log types:
    ///< `0`: White (info) log message
    ///< `1`: Yellow (warning) log message
    ///< '2': Red (error) log message
} ABI_SERVER_CORE_FUNCTIONS;


typedef struct ABI_MODULE_FUNCTIONS ABI_MODULE_FUNCTIONS; ///< Forward declaration

/**
 * @internal
 * @brief Functions of the module that calls the server core
 *
 * @warning If functions that return a pointer return `NULL`, it means that an error has occurred.
 *
 * @note This is a packaged structure.
 * @endinternal
 */
typedef struct __attribute__((packed)) ABI_MODULE_FUNCTIONS {
    ABI_MODULE_FUNCTIONS *(*module_init)(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                         const char *json_configuration); ///< Initializes module

    void (*module_destroy)(void); ///< Destroys module

    const char *(*module_get_configuration)(void); ///< Get module json configuration

    ABI_MODULE_MDTP_DATA *(*module_get_data)(void); ///< Get MDTP module data

    void (*module_enable)(void); ///< Enables a module

    void (*module_disable)(void); ///< Disables a module

    uint8_t (*module_is_enabled)(void); ///< Is module enabled (true/false)

    const char *(*module_get_module_name)(void); ///< Get module name

    const char *(*module_get_module_description)(void); ///< Get module description

    ABI_MODULE_CONTEXT *(*module_get_context)(void); ///< Get context of module. See `ABI_CONTEXT`
} ABI_MODULE_FUNCTIONS;


#ifdef __cplusplus
}
#endif
