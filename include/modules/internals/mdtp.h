/**
 * @file modules/internals/mdtp.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#define MDTP_VERSION 1 ///< MDTP version

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IModule              IModule;              ///< Forward declaration
typedef struct ABI_MODULE_MDTP_DATA ABI_MODULE_MDTP_DATA; ///< Forward declaration

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
 * @warning The strings are copied into the node. The caller remains responsible for freeing
 * them if allocated dynamically.
 * @warning Value nodes cannot contain children. They are always leaves in the MDTP structure.
 * @note If you want to generate a frame ready for transmission to the server, pass the pointer
 * obtained after calling this function to `sdk_mdtp_make_root`
 *
 * @return `void*` Pointer to the created value node. **Must be freed with
 * `sdk_mdtp_free_value` if you do not pass this pointer to `sdk_mdtp_make_container` or
 * `sdk_mdtp_make_root`**.
 *
 * @code{.c}
 * // Example usage:
 * void *val = sdk_mdtp_make_value("RAM", "1234", "MB");
 * // Do something with value...
 * sdk_mdtp_free_value(val);
 * @endcode
 */
void *sdk_mdtp_make_value(const char *value_name, const char *value, const char *value_units);

/**
 * @brief Frees memory allocated for value node via `sdk_mdtp_make_value`
 * @param value_node Pointer to value node
 * @note If a node of a **other type** is passed, there will be no effect
 */
void sdk_mdtp_free_value(void *value_node);

/**
 * @brief Creates a container node.
 *
 * Containers can include both value nodes and other containers, allowing hierarchical
 * structures. The payload size is automatically calculated from the nested nodes.
 *
 * @param name  Name of the container (non-NULL, zero-terminated string).
 *              Length is stored without the terminating `\0`.
 * @param first Pointer to the first nested node (container or value).
 * @param ...   Additional nested nodes. The list must always be terminated with `NULL`.
 *
 * @warning **Always** terminate the argument list with `NULL`.
 * @warning The function takes ownership of nested nodes and frees them when the container is
 * destroyed.
 * @warning The `name` string is copied into the container. The caller remains responsible for
 * freeing it if allocated dynamically.
 * @note If you want to generate a frame ready for transmission to the server, pass the pointer
 * obtained after calling this function to `sdk_mdtp_make_root`
 *
 * @return `void*` Pointer to the created container node. **Must be freed with
 * `MDTP_UTILS::free_container()` if you do not pass this pointer to `sdk_mdtp_make_container` or
 * `sdk_mdtp_make_root`**.
 *
 * @code{.c}
 * // Example usage:
 * void *container = sdk_mdtp_make_container("ram",
 *     sdk_mdtp_make_value("use", "12", "gb"),
 *     NULL);
 * // Do something with container...
 * sdk_mdtp_free_container(container);
 * @endcode
 */
void *sdk_mdtp_make_container(const char *name, void *first, ...);

/**
 * @brief Frees memory allocated for container node via `MDTP_UTILS::make_container`
 * @param value_node Pointer to container node
 * @note If a node of a **other type** is passed, there will be no effect
 */
void sdk_mdtp_free_container(void *container_node);

/**
 * @brief Generates a valid MDTP frame with header, ready to be sent to the server.
 *
 * This function creates the root frame of the MDTP protocol.
 * It accepts a variable number of nodes (containers or values) and packs them into a root
 * object. The result can be directly sent to the server since the frame is complete and valid.
 *
 * @param module The module in which the data will be saved
 * @param first Pointer to the first nested node (container or value). Must not be `NULL`.
 * @param ...   Additional nodes. The list must always be terminated with `NULL`.
 *
 * @warning Always terminate the argument list with `NULL`.
 * @warning The function takes ownership of passed nodes. Do not free them manually afterwards.
 *
 * @return Pointer to a valid `SDK_MODULE_MDTP_DATA` frame. **Do not free it, as this will
 * happen automatically when the module terminates!**
 *
 * @code{.c}
 * // Example usage:
 * SDK_MODULE_MDTP_DATA *data = sdk_mdtp_make_root(module,
 *     sdk_mdtp_make_container("ram",
 *         sdk_mdtp_make_value("usage", "12", "gb"),
 *         NULL),
 *     NULL);
 * @endcode
 */
const ABI_MODULE_MDTP_DATA *sdk_mdtp_make_root(IModule *module, void *first, ...);


#ifdef __cplusplus
}
#endif
