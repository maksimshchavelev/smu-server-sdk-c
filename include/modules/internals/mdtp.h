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

typedef struct ABI_MODULE_MDTP_DATA ABI_MODULE_MDTP_DATA; ///< Forward declaration

/**
 * @brief Structure with pointers to functions for working with the MDTP protocol
 */
typedef struct MDTP_UTILS {
    /**
     * @internal
     * @brief Generates a valid MDTP frame with header, ready to be sent to the server.
     *
     * This function creates the root frame of the MDTP protocol.
     * It accepts a variable number of nodes (containers or values) and packs them into a root
     * object. The result can be directly sent to the server since the frame is complete and valid.
     *
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
     * SDK_MODULE_MDTP_DATA *data = sdk_mdtp_make_root(
     *     sdk_mdtp_make_container("ram",
     *         sdk_mdtp_make_value("use", "12", "gb"),
     *         NULL),
     *     NULL);
     * @endcode
     * @endinternal
     */
    ABI_MODULE_MDTP_DATA *(*make_root)(void *first, ...);

    /**
     * @internal
     * @brief Creates a container node.
     *
     * Containers can include both value nodes and other containers, allowing hierarchical
     * structures. The payload size is automatically calculated from the nested nodes.
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
     * @endinternal
     */
    void *(*make_container)(const char *name, void *first, ...);

    /**
     * @internal
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
     *
     * @return `void*` Pointer to the created value node. **Must be freed with
     * `sdk_mdtp_free_value()`**.
     *
     * @code{.c}
     * // Example usage:
     * void *val = sdk_mdtp_make_value("RAM", "1234", "MB");
     * @endcode
     * @endinternal
     */
    void *(*make_value)(const char *value_name, const char *value, const char *value_units);

    /**
     * @internal
     * @brief Frees memory allocated for container node
     * @param value_node Pointer to container node
     * @note If a node of a other type is passed, there will be no effect
     * @endinternal
     */
    void (*free_container)(void *container_node);

    /**
     * @internal
     * @brief Frees memory allocated for value node
     * @param value_node Pointer to value node
     * @note If a node of a other type is passed, there will be no effect
     * @endinternal
     */
    void (*free_value)(void *value_node);
} MDTP_UTILS;


/**
 * @internal
 * @brief Creates an `MDTP_UTILS` object and returns it
 * @warning Always call `mdtp_utils_destroy` to free resources after finishing with MDTP!
 * @return `MDTP_UTILS`
 * @endinternal
 */
MDTP_UTILS mdtp_utils_init(void);


/**
 * @internal
 * @brief Destroys internal resources allocated during MDTP operation.
 * @param mdtp `MDTP_UTILS` object obtained after calling `mdtp_utils_init`
 * @endinternal
 */
void mdtp_utils_destroy(MDTP_UTILS mdtp);


#ifdef __cplusplus
}
#endif
