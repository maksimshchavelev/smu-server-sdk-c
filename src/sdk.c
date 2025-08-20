/**
 * @file sdk.c
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "sdk.h"                     // For public interface
#include "internals/memutils.h"      // For memory utilities
#include "internals/sdk_internals.h" // For SDK internals
#include "version.h"
#include <stdlib.h>
#include <string.h>


static SDK_MODULE_ABI_CONTEXT module_context_storage; ///< For context
static SDK_MODULE_MDTP_DATA
    module_mdtp_data_storage; ///< For temporary storage of MDTP protocol data
static SDK_ABI_SERVER_CORE_FUNCTIONS
    abi_server_core_functions_storage;                    ///< For pointers to server functions
static SDK_ABI_MODULE_FUNCTIONS module_functions_storage; ///< For pointer to module functions


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

    module_context = &module_context_storage; // Init pointer

    module_context->module_name = strdup(module_name);        // Free after use
    module_context->module_description = strdup(module_name); // Free after use

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
    memcpy(buffer + offset, value_name, value_name_length);
    offset += value_name_length;

    // Write units length
    write_uint32_be(buffer, offset, (uint32_t)value_units_length);
    offset += 4;

    // Write units
    memcpy(buffer + offset, value_units, value_units_length);
    offset += value_units_length;

    // Write value length
    write_uint32_be(buffer, offset, (uint32_t)value_length);
    offset += 4;

    // Write value
    memcpy(buffer + offset, value, value_length);

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

    abi_server_core_functions = &abi_server_core_functions_storage;
    *abi_server_core_functions = server_functions;

    sdk_utils = (SDK_UTILS){.log = sdk_utils_log,
                            .module_setup = sdk_utils_module_setup,
                            .get_module_name = sdk_utils_get_module_name,
                            .get_module_description = sdk_utils_get_module_name,
                            .get_module_context = sdk_utils_get_module_context};

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
