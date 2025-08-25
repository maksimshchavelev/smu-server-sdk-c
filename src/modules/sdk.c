/**
 * @file sdk.c
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "modules/sdk.h"                     // For public interface
#include "modules/internals/sdk_internals.h" // For SDK internals
#include "version.h"
#include <stdarg.h> // For variable arguments
#include <stdlib.h>
#include <string.h>


static ABI_MODULE_CONTEXT        module_context_storage = {0}; ///< For context
static ABI_SERVER_CORE_FUNCTIONS abi_server_core_functions_storage = {
    0};                                                     ///< For pointers to server functions
static ABI_MODULE_FUNCTIONS module_functions_storage = {0}; ///< For pointer to module functions


// We use pointers to understand whether structures are initialized or not. malloc is not used and
// free does not need to be called.
// Note: set to NULL when module destroys
static ABI_MODULE_CONTEXT        *module_context = NULL; ///< Pointer to context
static ABI_SERVER_CORE_FUNCTIONS *abi_server_core_functions =
    NULL; ///< Pointer to structure with server functions
static ABI_MODULE_FUNCTIONS *module_functions =
    NULL; ///< Pointer to structure with module functions


static SDK_UTILS sdk_utils; ///< For sdk utility functions


// =================================== OTHER FUNCTIONS ===================================

// Get SDK_UTILS instance
SDK_ABI SDK_UTILS *sdk_utils_get(void) {
    return &sdk_utils;
}


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
ABI_MODULE_CONTEXT *sdk_utils_get_module_context(void) {
    return module_context;
}


// ========================== ABI IMPLEMENTATION ==========================


// Init module. Calls sdk_module_init implemented by user
SDK_ABI ABI_MODULE_FUNCTIONS *module_init(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                          const char               *json_configuration) {
    ABI_MODULE_CONTEXT temporary_context = (ABI_MODULE_CONTEXT){
        .module_name = "anonymously",
        .module_description = "anonymously"
    };

    // Check ABI version
    if (server_functions.abi_get_abi_version(&temporary_context /* anonymously context */) != ABI_VERSION) {
        return NULL; // Incorrect ABI version
    }

    if (json_configuration == NULL) {
        return NULL; // error
    }

    module_context = &module_context_storage; // Init pointer

    abi_server_core_functions = &abi_server_core_functions_storage;
    *abi_server_core_functions = server_functions;


    sdk_utils = (SDK_UTILS){.log = sdk_utils_log,
                            .module_setup = sdk_utils_module_setup,
                            .get_module_name = sdk_utils_get_module_name,
                            .get_module_description = sdk_utils_get_module_description,
                            .get_module_context = sdk_utils_get_module_context,
                            // Init MDTP utils
                            .mdtp = mdtp_utils_init()};

    // Call sdk_module_init
    if (sdk_module_init(json_configuration) != SDK_OK) {
        return NULL; // error
    }

    module_functions = &module_functions_storage;
    *module_functions = (ABI_MODULE_FUNCTIONS){
        .module_init = module_init,
        .module_destroy = module_destroy,
        .module_get_configuration = sdk_module_get_configuration,
        .module_get_data = sdk_module_get_data,
        .module_enable = sdk_module_enable,
        .module_disable = sdk_module_disable,
        .module_is_enabled = sdk_module_is_enabled,
        .module_set_poll_ratio = sdk_module_set_poll_ratio,
        .module_get_poll_ratio = sdk_module_get_poll_ratio,
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
    mdtp_utils_destroy(sdk_utils.mdtp);
}
