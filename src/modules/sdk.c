/**
 * @file sdk.c
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "modules/sdk.h"                     // For public interface
#include "modules/internals/memutils.h"      // For memory utilities
#include "modules/internals/sdk_internals.h" // For SDK internals
#include "version.h"
#include <stdarg.h> // For variable arguments
#include <stdlib.h>
#include <string.h>


static SDK_MODULE_ABI_CONTEXT module_context_storage = (SDK_MODULE_ABI_CONTEXT){0}; ///< For context

static SDK_MODULE_MDTP_DATA module_mdtp_data_storage =
    (SDK_MODULE_MDTP_DATA){0}; ///< For temporary storage of MDTP protocol data

static SDK_ABI_SERVER_CORE_FUNCTIONS abi_server_core_functions_storage =
    (SDK_ABI_SERVER_CORE_FUNCTIONS){0}; ///< For pointers to server functions

static SDK_ABI_MODULE_FUNCTIONS module_functions_storage =
    (SDK_ABI_MODULE_FUNCTIONS){0}; ///< For pointer to module functions


// We use pointers to understand whether structures are initialized or not. malloc is not used and
// free does not need to be called.
// Note: set to NULL when module destroys
static SDK_MODULE_ABI_CONTEXT *module_context = NULL; ///< Pointer to context
static SDK_MODULE_MDTP_DATA *module_mdtp_data = NULL; ///< Pointer to storage of MDTP protocol data
static SDK_ABI_SERVER_CORE_FUNCTIONS *abi_server_core_functions =
    NULL; ///< Pointer to structure with server functions
static SDK_ABI_MODULE_FUNCTIONS *module_functions =
    NULL; ///< Pointer to structure with module functions


SDK_UTILS sdk_utils; ///< For sdk utility functions


// ========================== SDK HELPER FUNCTIONS IMPLEMENTATION ==========================
// ================================== USE VIA sdk_utils ====================================


// Setup context
SDKStatus sdk_utils_module_setup(const char *module_name, const char *module_description) {
    if (module_name == NULL || module_description == NULL || strlen(module_name) == 0 ||
        strlen(module_description) == 0) {
        return SDK_INVALID_ARGUMENT;
    }

    module_context->module_name = strdup(module_name);               // Free after use
    module_context->module_description = strdup(module_description); // Free after use

    if (module_context->module_name == NULL || module_context->module_description == NULL) {
        return SDK_ALLOCATION_ERROR;
    }

    return SDK_OK;
}


// Logger function
SDKStatus sdk_utils_log(LogType log_type, const char *message) {
    if (message == NULL) {
        return SDK_INVALID_ARGUMENT;
    }

    if (module_context == NULL || abi_server_core_functions == NULL) {
        return SDK_INTERNALS_UNINITIALIZED;
    }

    abi_server_core_functions->abi_log(module_context, (int)log_type, message);

    return SDK_OK;
}


// Get module description
const char *sdk_utils_get_module_name(void) {
    return module_context->module_name;
}


// Get module description
const char *sdk_utils_get_module_description(void) {
    return module_context->module_description;
}


// Get module context
SDK_MODULE_ABI_CONTEXT *sdk_utils_get_module_context(void) {
    return module_context;
}



// ========================== MDTP HELPER FUNCTIONS IMPLEMENTATION ==========================
// ================================ USE VIA sdk_utils.mdtp ==================================

// Make value node
void *sdk_mdtp_make_value(const char *value_name, const char *value, const char *value_units) {
    // From MDTP v1 specification:

    // [node type]: 1 unsigned byte (1 because node is value)
    // [node name length]: unsigned int32
    // [name of node...]: array of char
    // [units length]: unsigned int32
    // [units...]: array of char
    // [value length]: unsigned int32
    // [value...]: array of char

    if (value_name == NULL || value == NULL || value_units == NULL) {

        return NULL;
    }

    size_t buffer_size = 1 + sizeof(uint32_t) + strlen(value_name) + sizeof(uint32_t) +
                         strlen(value_units) + sizeof(uint32_t) + strlen(value);
    size_t offset = 0; // Current position

    void *buffer = calloc(1, buffer_size); // Use calloc to fill memory by zeroes

    // Allocation error
    if (buffer == NULL) {
        return NULL;
    }

    size_t value_name_length = strlen(value_name);
    size_t value_units_length = strlen(value_units);
    size_t value_length = strlen(value);

    // Write node type
    write_ubyte_be(buffer, offset, 1);
    ++offset;

    // Write node name length
    write_uint32_be(buffer, offset, (uint32_t)value_name_length);
    offset += 4;

    // Write node name
    memcpy((char *)buffer + offset, value_name, value_name_length);
    offset += value_name_length;

    // Write units length
    write_uint32_be(buffer, offset, (uint32_t)value_units_length);
    offset += 4;

    // Write units
    memcpy((char *)buffer + offset, value_units, value_units_length);
    offset += value_units_length;

    // Write value length
    write_uint32_be(buffer, offset, (uint32_t)value_length);
    offset += 4;

    // Write value
    memcpy((char *)buffer + offset, value, value_length);

    return buffer;
}


// Free value node
void sdk_mdtp_free_value(void *value_node) {
    // If not value node
    if (read_ubyte_be(value_node, 0) != 1) {
        return;
    }

    free(value_node);
}


// Make container node
void *sdk_mdtp_make_container(const char *name, void *first, ...) {
    if (name == NULL || first == NULL) {
        return NULL;
    }

    va_list args;
    va_list args_copy;
    va_start(args, first);
    va_copy(args_copy, args);

    void    *ptr = first;
    void    *buffer;
    size_t   offset = 0;
    size_t   name_length = strlen(name);
    uint32_t payload_size = sdk_mdtp_get_nodes_size_va(first, args_copy);

    uint32_t size = 1 /* node type */ + 4 /* name length */ + (uint32_t)name_length /* name */ +
                    4 /* payload size */ + payload_size /* payload */;
    buffer = calloc(1, size); // calloc to fill memory by zeroes

    if (buffer == NULL) {
        va_end(args);
        va_end(args_copy);
        return NULL;
    }

    // Fill buffer
    // Write node type
    write_ubyte_be(buffer, offset, 0 /* container */);
    ++offset;

    // Write name length
    write_uint32_be(buffer, offset, (uint32_t)name_length);
    offset += 4;

    // Write name
    memcpy((char *)buffer + offset, name, name_length);
    offset += name_length;

    // Write payload size
    write_uint32_be(buffer, offset, payload_size);
    offset += 4;

    // Write payload
    while (ptr != NULL) {
        uint32_t node_size = sdk_mdtp_get_nodes_size(ptr, NULL);
        memcpy((char *)buffer + offset, ptr, node_size);

        // If node type is value
        if (read_ubyte_be(ptr, 0) == 1) {
            sdk_mdtp_free_value(ptr);
        }
        // If node type is container
        else if (read_ubyte_be(ptr, 0) == 0) {
            sdk_mdtp_free_container(ptr);
        }

        offset += node_size;

        ptr = va_arg(args, void *);
    }

    va_end(args);
    va_end(args_copy);

    return buffer;
}


// Free container node
void sdk_mdtp_free_container(void *container_node) {
    // If not container node
    if (read_ubyte_be(container_node, 0) != 0) {
        return;
    }

    free(container_node);
}


// Make root node
SDK_MODULE_MDTP_DATA *sdk_mdtp_make_root(void *first, ...) {
    va_list args;
    va_list args_copy;
    va_start(args, first);
    va_copy(args_copy, args);

    void    *ptr = first;
    size_t   offset = 0;
    uint32_t payload_size = sdk_mdtp_get_nodes_size_va(first, args_copy);

    uint32_t size = 1 /* version of MDTP */ + 4 /* payload size */ + payload_size /* payload */;
    free((void *)module_mdtp_data->data);
    module_mdtp_data->data = calloc(1, size); // calloc to fill memory by zeroes
    module_mdtp_data->size = size;

    // Fill buffer
    // Write MDTP version
    write_ubyte_be((void *)module_mdtp_data->data, offset, MDTP_VERSION);
    ++offset;

    // Write payload size
    write_uint32_be((void *)module_mdtp_data->data, offset, payload_size);
    offset += 4;

    // Write payload
    while (ptr != NULL) {
        uint32_t node_size = sdk_mdtp_get_nodes_size(ptr, NULL);
        memcpy((char *)module_mdtp_data->data + offset, ptr, node_size);

        // If node type is value
        if (read_ubyte_be(ptr, 0) == 1) {
            sdk_mdtp_free_value(ptr);
        }
        // If node type is container
        else if (read_ubyte_be(ptr, 0) == 0) {
            sdk_mdtp_free_container(ptr);
        }

        offset += node_size;

        ptr = va_arg(args, void *);
    }

    va_end(args);
    va_end(args_copy);

    return module_mdtp_data;
}


// Get node size
uint32_t sdk_mdtp_get_nodes_size(void *first, ...) {
    va_list args;
    va_start(args, first);
    uint32_t size = sdk_mdtp_get_nodes_size_va(first, args);
    va_end(args);
    return size;
}


// Get nodes size via va_list
uint32_t sdk_mdtp_get_nodes_size_va(void *first, va_list args) {
    if (first == NULL) {
        return 0u;
    }

    uint64_t total = 0; // accumulate in 64-bit to avoid intermediate overflow

    for (void *p = first; p != NULL; p = va_arg(args, void *)) {
        const uint8_t *b = (const uint8_t *)p;

        // Read node type (first byte)
        uint8_t type = b[0];

        if (type == 1) {
            /* VALUE NODE:
             * [1 type] [4 name_len] [name] [4 units_len] [units] [4 value_len] [value]
             */
            size_t off = 1;

            uint32_t name_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)name_len;

            uint32_t units_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)units_len;

            uint32_t value_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)value_len;

            total += off;

        } else if (type == 0) {
            /* CONTAINER NODE:
             * [1 type] [4 name_len] [name] [4 payload_size] [payload...]
             */
            size_t off = 1;

            uint32_t name_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)name_len;

            uint32_t payload_size = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)payload_size;

            total += off;

        } else {
            // Unknown type: ignore (adds 0). Optionally you can choose to bail out.
        }
    }

    // Clamp to UINT32_MAX if sum exceeded 32-bit range
    return (total > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (uint32_t)total;
}

// ========================== ABI IMPLEMENTATION ==========================


// Init module. Calls sdk_module_init implemented by user
SDK_ABI SDK_ABI_MODULE_FUNCTIONS *module_init(SDK_ABI_SERVER_CORE_FUNCTIONS server_functions,
                                              const char                   *json_configuration) {
    // Check ABI version
    if (server_functions.abi_get_abi_version(NULL /* no context yet */) != ABI_VERSION) {
        return NULL; // Incorrect ABI version
    }

    if (json_configuration == NULL) {
        return NULL; // error
    }

    module_context = &module_context_storage; // Init pointer

    abi_server_core_functions = &abi_server_core_functions_storage;
    *abi_server_core_functions = server_functions;

    module_mdtp_data = &module_mdtp_data_storage;

    sdk_utils = (SDK_UTILS){.log = sdk_utils_log,
                            .module_setup = sdk_utils_module_setup,
                            .get_module_name = sdk_utils_get_module_name,
                            .get_module_description = sdk_utils_get_module_name,
                            .get_module_context = sdk_utils_get_module_context,
                            // Init MDTP utils
                            .mdtp = (MDTP){.make_value = sdk_mdtp_make_value,
                                           .make_container = sdk_mdtp_make_container,
                                           .make_root = sdk_mdtp_make_root,
                                           .free_value = sdk_mdtp_free_value,
                                           .free_container = sdk_mdtp_free_container}};

    // Call sdk_module_init
    if (sdk_module_init(json_configuration) != SDK_OK) {
        return NULL; // error
    }

    module_functions = &module_functions_storage;
    *module_functions = (SDK_ABI_MODULE_FUNCTIONS){
        .module_init = module_init,
        .module_destroy = module_destroy,
        .module_get_configuration = sdk_module_get_configuration,
        .module_get_data = sdk_module_get_data,
        .module_enable = sdk_module_enable,
        .module_disable = sdk_module_disable,
        .module_is_enabled = sdk_module_is_enabled,
        .module_get_module_name = sdk_utils.get_module_name,
        .module_get_module_description = sdk_utils.get_module_description,
        .module_get_context = sdk_utils.get_module_context};

    return module_functions;
}


// Destroy module
SDK_ABI void module_destroy(void) {
    sdk_module_destroy();

    // Free resources
    free((void *)module_context->module_name);
    free((void *)module_context->module_description);
    free((void *)module_mdtp_data->data);
}
