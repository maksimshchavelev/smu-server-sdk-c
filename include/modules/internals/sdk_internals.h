/**
 * @file internals/sdk_internals.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include <stdint.h>
#include <stdio.h>

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


// =========================== SDK MDTP FUNCTIONS ===========================

/**
 * @brief Generates a valid MDTP frame with header, ready to be sent to the server.
 *
 * This function creates the root frame of the MDTP protocol.
 * It accepts a variable number of nodes (containers or values) and packs them into a root object.
 * The result can be directly sent to the server since the frame is complete and valid.
 *
 * @param first Pointer to the first nested node (container or value). Must not be `NULL`.
 * @param ...   Additional nodes. The list must always be terminated with `NULL`.
 *
 * @warning Always terminate the argument list with `NULL`.
 * @warning The function takes ownership of passed nodes. Do not free them manually afterwards.
 *
 * @return Pointer to a valid `SDK_MODULE_MDTP_DATA` frame. **Do not free it, as this will happen
 * automatically when the module terminates!**
 *
 * @code{.c}
 * // Example usage:
 * SDK_MODULE_MDTP_DATA *data = sdk_mdtp_make_root(
 *     sdk_mdtp_make_container("ram",
 *         sdk_mdtp_make_value("use", "12", "gb"),
 *         NULL),
 *     NULL);
 * @endcode
 */
SDK_MODULE_MDTP_DATA *sdk_mdtp_make_root(const void *first, ...);


/**
 * @brief Creates a container node.
 *
 * Containers can include both value nodes and other containers, allowing hierarchical structures.
 * The payload size is automatically calculated from the nested nodes.
 *
 * @param name  Name of the container (non-NULL, zero-terminated string).
 *              Length is stored without the terminating `\0`.
 * @param first Pointer to the first nested node (container or value). May be `NULL` if the
 * container is empty.
 * @param ...   Additional nested nodes. The list must always be terminated with `NULL`.
 *
 * @warning Always terminate the argument list with `NULL`.
 * @warning The function takes ownership of nested nodes and frees them when the container is
 * destroyed.
 * @warning The `name` string is copied into the container. The caller remains responsible for
 * freeing it if allocated dynamically.
 *
 * @return `void*` Pointer to the created container node. **Must be freed with
 * `sdk_mdtp_free_container()`**.
 *
 * @code{.c}
 * // Example usage:
 * void *container = sdk_mdtp_make_container("ram",
 *     sdk_mdtp_make_value("use", "12", "gb"),
 *     NULL);
 * @endcode
 */
void *sdk_mdtp_make_container(const char *name, const void *first, ...);


/**
 * @brief Frees memory allocated for container node
 * @param value_node Pointer to container node
 * @note If a node of a other type is passed, there will be no effect
 */
void sdk_mdtp_free_container(const void *container_node);


/**
 * @brief Creates a value node.
 *
 * A value node holds a name, a measurement unit, and a value string.
 * All strings are stored without the terminating `\0`.
 *
 * @param value_name  Name of the value (non-NULL, zero-terminated string).
 * @param value       Value string (non-NULL, zero-terminated string).
 * @param value_units Units string (non-NULL, zero-terminated string).
 *
 * @warning The strings are copied into the node. The caller remains responsible for freeing them if
 * allocated dynamically.
 * @warning Value nodes cannot contain children. They are always leaves in the MDTP structure.
 *
 * @return `void*` Pointer to the created value node. **Must be freed with
 * `sdk_mdtp_free_value()`**.
 *
 * @code{.c}
 * // Example usage:
 * void *val = sdk_mdtp_make_value("RAM", "1234", "MB");
 * @endcode
 */
void *sdk_mdtp_make_value(const char *value_name, const char *value, const char *value_units);


/**
 * @brief Frees memory allocated for value node
 * @param value_node Pointer to value node
 * @note If a node of a other type is passed, there will be no effect
 */
void sdk_mdtp_free_value(const void *value_node);


/**
 * @brief Computes the total serialized size (in bytes) of one or more MDTP nodes.
 *
 * This function accepts a variable number of pointers to **serialized MDTP nodes in memory**
 * (wire layout), determines the node type from the first byte, parses the big-endian length
 * fields according to the MDTP specification, and returns the sum of sizes for all nodes.
 * The argument list MUST be terminated with NULL.
 *
 * ## Assumptions
 * - Each pointer refers to the **serialized** node layout starting at the node's first byte:
 *   - **Container node** layout:
 *     @code
 *     [1 byte type=0]
 *     [4 bytes name_len (BE)]
 *     [name bytes (name_len)]
 *     [4 bytes payload_size (BE)]
 *     [payload bytes (payload_size)]
 *     @endcode
 *   - **Value node** layout:
 *     @code
 *     [1 byte type=1]
 *     [4 bytes name_len (BE)]
 *     [name bytes (name_len)]
 *     [4 bytes units_len (BE)]
 *     [units bytes (units_len)]
 *     [4 bytes value_len (BE)]
 *     [value bytes (value_len)]
 *     @endcode
 * - All 32-bit integers are encoded **big-endian**.
 * - Input pointers must be valid and point to buffers large enough for the declared lengths.
 * - The function does **not** validate contents beyond basic parsing; malformed nodes yield
 *   undefined behavior (or truncated size if arithmetic overflows).
 *
 * ## Overflow behavior
 * - The function accumulates sizes in 64-bit and clamps the final result to UINT32_MAX.
 *
 * ## Variadic termination
 * - The list MUST end with `NULL`, e.g.:
 *   @code
 *   uint32_t sum = sdk_mdtp_get_nodes_size(nodeA, nodeB, NULL);
 *   @endcode
 *
 * @param first Pointer to the first serialized node (container or value). Pass NULL for zero nodes.
 * @param ...   More node pointers, terminated with NULL.
 * @return The total size in bytes (clamped to UINT32_MAX).
 *
 * @section example_value Example: single value node
 * @code
 * // value node for: name="version", units="", value="1.0.42"
 * // Layout: [1][4][name][4][units][4][value]
 * const uint8_t value_node[] = {
 *   0x01,                           // type = value
 *   0x00,0x00,0x00,0x07,            // name_len = 7
 *   'v','e','r','s','i','o','n',    // name
 *   0x00,0x00,0x00,0x00,            // units_len = 0
 *   0x00,0x00,0x00,0x06,            // value_len = 6
 *   '1','.','0','.','4','2'         // value
 * };
 * uint32_t sz = sdk_mdtp_get_nodes_size((void*)value_node, NULL);
 * // sz == 1 + 4 + 7 + 4 + 0 + 4 + 6 = 26 bytes
 * @endcode
 *
 * @section example_container Example: container with payload
 * @code
 * // Suppose 'payload' already contains two serialized value nodes, total 55 bytes.
 * extern const uint8_t payload[55];
 * const uint8_t container_node[] = {
 *   0x00,                         // type = container
 *   0x00,0x00,0x00,0x03,          // name_len = 3
 *   'c','p','u',                  // name
 *   0x00,0x00,0x00,0x37,          // payload_size = 55
 *   // ... 55 bytes of payload follow ...
 * };
 * uint32_t sz = sdk_mdtp_get_nodes_size((void*)container_node, NULL);
 * // sz == 1 + 4 + 3 + 4 + 55 = 67 bytes
 * @endcode
 */
uint32_t sdk_mdtp_get_nodes_size(const void *first, ...);


/**
 * @brief Calculates the total size of MDTP nodes using va_list.
 * @param first Pointer to the first node.
 * @param args  Variadic argument list with subsequent nodes, ending with NULL.
 * @return Total size of all nodes in bytes.
 * @warning **Copy the `va_list` instance via `va_copy` and pass the copy along!**
 */
uint32_t sdk_mdtp_get_nodes_size_va(const void *first, va_list args);


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
