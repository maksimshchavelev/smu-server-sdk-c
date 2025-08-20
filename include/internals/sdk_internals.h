/**
 * @file internals/sdk_internals.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include <stdint.h>

#if defined _WIN32
#define SDK_ABI __declspec(dllexport)
#else
#define SDK_ABI __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Context of module
 *
 * The context is needed so that the server core knows what module is accessing it (when calling
 * core functions, for example, logging)
 *
 * @note This is a **packaged** structure
 */
typedef struct __attribute__((packed)) SDK_MODULE_ABI_CONTEXT {
    const char *module_name;        ///< Name of module
    const char *module_description; ///< Description of module
} SDK_MODULE_ABI_CONTEXT;


/**
 * @brief Struct to storing MDTP data. See documentation for MDTP protocol.
 * @note This is a **packaged** structure.
 */
typedef struct __attribute__((packed)) SDK_MODULE_MDTP_DATA {
    const uint8_t *data; ///< Pointer to MDTP bytes
    uint32_t       size; ///< Count of MDTP bytes (size of data)
} SDK_MODULE_MDTP_DATA;


/**
 * @brief Functions provided by the server for the module
 *
 * This structure will be passed to the module initialization function.
 *
 * @note This is a packaged structure.
 */
typedef struct __attribute__((packed)) SDK_ABI_SERVER_CORE_FUNCTIONS {
    int (*abi_get_abi_version)(SDK_MODULE_ABI_CONTEXT *context); ///< Returns ABI version

    void (*abi_log)(SDK_MODULE_ABI_CONTEXT *context,
                    int                     log_type,
                    const char             *message); ///< Logging function. Log types:
    ///< `0`: White (info) log message
    ///< `1`: Yellow (warning) log message
    ///< '2': Red (error) log message
} SDK_ABI_SERVER_CORE_FUNCTIONS;


typedef struct SDK_ABI_MODULE_FUNCTIONS SDK_ABI_MODULE_FUNCTIONS; ///< Forward declaration

/**
 * @brief Functions of the module that calls the server core
 *
 * @warning If functions that return a pointer return `NULL`, it means that an error has occurred.
 *
 * @note This is a packaged structure.
 */
typedef struct __attribute__((packed)) SDK_ABI_MODULE_FUNCTIONS {
    SDK_ABI_MODULE_FUNCTIONS *(*module_init)(
        SDK_ABI_SERVER_CORE_FUNCTIONS server_functions,
        const char                   *json_configuration); ///< Initializes module

    void (*module_destroy)(void); ///< Destroys module

    const char *(*module_get_configuration)(void); ///< Get module json configuration

    SDK_MODULE_MDTP_DATA *(*module_get_data)(void); ///< Get MDTP module data

    void (*module_enable)(void); ///< Enables a module

    void (*module_disable)(void); ///< Disables a module

    uint8_t (*module_is_enabled)(void); ///< Is module enabled (true/false)

    const char *(*module_get_module_name)(void); ///< Get module name

    const char *(*module_get_module_description)(void); ///< Get module description

    SDK_MODULE_ABI_CONTEXT *(*module_get_context)(
        void); ///< Get context of module. See `ABI_CONTEXT`
} SDK_ABI_MODULE_FUNCTIONS;


// =========================== ABI FUNCTIONS ===========================

/**
 * @brief Initialize the module. Entry function
 * @param server_functions Struct with server functions
 * @param json_configuration Json configuration
 * @return Pointer to `SDK_ABI_MODULE_FUNCTIONS` with module functions
 */
SDK_ABI SDK_ABI_MODULE_FUNCTIONS *module_init(SDK_ABI_SERVER_CORE_FUNCTIONS server_functions,
                                              const char                   *json_configuration);

/**
 * @brief Calls `sdk_module_destroy` and frees resources
 */
SDK_ABI void module_destroy(void);


// =========================== SDK STRUCTURES ===========================

/**
 * @brief MDTP container node. Read about MDTP protocol in documentation
 */
typedef struct MDTP_CONTAINER_NODE {
    uint8_t     node_type;        ///< `0` - if node type is container, `1` if node type is value
    uint32_t    node_name_length; ///< Length of node name without terminating zero
    const char *node_name;        ///< Name of node without terminating zero
    uint32_t    payload_size;     ///< Size of payload
    void       *payload; ///< Payload. Determine the node type and cast to the required type
} MDTP_CONTAINER_NODE;


/**
 * @brief MDTP value node. Read about MDTP protocol in documentation
 */
typedef struct MDTP_VALUE_NODE {
    uint8_t     node_type;        ///< `0` - if node type is container, `1` if node type is value
    uint32_t    node_name_length; ///< Length of node name without terminating zero
    const char *node_name;        ///< Name of node without terminating zero
    uint32_t    units_length;     ///< Length of node units without terminating zero
    const char *units;            ///< Units of node without terminating zero
    uint32_t    value_length;     ///< Length of value without terminating zero
    const char *value;            ///< Value of node without terminating zero
} MDTP_VALUE_NODE;



// =========================== SDK MDTP FUNCTIONS ===========================

/**
 * @brief Generates a valid MDTP frame with header, ready to be sent to the server
 * @return Valid MDTP frame
 */
SDK_MODULE_MDTP_DATA *sdk_mdtp_make_root(void *first, ...);


/**
 * @brief Creates a container node. Accepts both other container nodes and value nodes.
 * @return `void*` pointer to container with nodes
 */
void *sdk_mdtp_make_container(void *first, ...);


/**
 * @brief Creates a value node
 * @param value_name Name of value
 * @param value Value
 * @param value_units Units of value
 * @return `void*` pointer to value node
 */
void *sdk_mdtp_make_value(const char *value_name, const char *value, const char *value_units);


/**
 * @brief Frees memory allocated for value node
 * @param value_node Pointer to value node
 */
void sdk_mdtp_free_value(void *value_node);


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
SDK_MODULE_ABI_CONTEXT *sdk_utils_get_module_context(void);


#ifdef __cplusplus
}
#endif
