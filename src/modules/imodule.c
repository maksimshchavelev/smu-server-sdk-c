/**
 * @file modules/internals/imodule.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "modules/internals/imodule.h"
#include <malloc.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IModule {
    ABI_MODULE_CONTEXT        context;          ///< Context of the module
    ABI_MODULE_MDTP_DATA      mdtp_data;        ///< Temporary buffer for MDTP data
    ABI_MODULE_FUNCTIONS      module_functions; ///< Module functions
    ABI_SERVER_CORE_FUNCTIONS server_functions; ///< Server core functions
    uint32_t                  poll_ratio;       ///< Poll ratio of the module
    uint8_t                   is_enabled;       ///< `1` if module is enabled, otherwise `0`
} IModule;


ABI_MODULE_FUNCTIONS module_init(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                 const char *json_configuration); ///< Forward declaration


// ================================== UTILS ==================================

// Allocate memory for module and initialize it
IModule *sdk_imodule_create(const char               *name,
                            const char               *description,
                            ABI_SERVER_CORE_FUNCTIONS server_functions,
                            uint32_t                  poll_ratio,
                            uint8_t                   is_enabled) {
    if (!name || !description || !strlen(name) || !strlen(description)) {
        return NULL; // error
    }

    IModule *module = malloc(sizeof(IModule));

    if (!module) {
        return NULL; // error
    }

    memset(module, 0x0, sizeof(IModule));

    // Setup functions
    module->module_functions.module_init = module_init;
    module->server_functions = server_functions;

    // Setup context
    module->context.module_name = strdup(name);
    module->context.module_description = strdup(description);
    if (!module->context.module_name || !module->context.module_description) {
        return NULL;
    }

    // Setup other parameters
    module->poll_ratio = poll_ratio;
    module->is_enabled = is_enabled;

    return module;
}


// Destroys module and deallocates memory allocated for module
void sdk_imodule_destroy(IModule *module) {
    if (!module) {
        return;
    }

    // Destroy context
    free((void *)module->context.module_name);
    free((void *)module->context.module_description);

    // Destroy MDTP data
    free((void *)module->mdtp_data.data);

    // Free memory
    free((void *)module);
}


// Get context of the module
const ABI_MODULE_CONTEXT *sdk_imodule_get_context(const IModule *module) {
    return &module->context;
}


// Set context of the module
void sdk_imodule_set_context(IModule *module, ABI_MODULE_CONTEXT context) {
    free((void *)module->context.module_name);
    free((void *)module->context.module_description);

    module->context = context;
}


// Get MDTP data of the module
const ABI_MODULE_MDTP_DATA *sdk_imodule_get_mdtp_data(const IModule *module) {
    return &module->mdtp_data;
}


// Set MDTP data of the module
void sdk_imodule_set_mdtp_data(IModule *module, ABI_MODULE_MDTP_DATA data) {
    free((void *)module->mdtp_data.data);
    module->mdtp_data = data;
}


// Get poll ratio of the module
uint32_t sdk_imodule_get_poll_ratio(const IModule *module) {
    return module->poll_ratio;
}


// Set poll ratio of the module
void sdk_imodule_set_poll_ratio(IModule *module, uint32_t poll_ratio) {
    module->poll_ratio = poll_ratio;
}


// Enable the module
void sdk_imodule_enable(IModule *module) {
    module->is_enabled = 1;
}


// Disables the module
void sdk_imodule_disable(IModule *module) {
    module->is_enabled = 0;
}


// Is module enabled?
uint8_t sdk_imodule_is_enabled(const IModule *module) {
    return module->is_enabled;
}


// Get module functions
const ABI_MODULE_FUNCTIONS *sdk_imodule_get_module_functions(const IModule *module) {
    return &module->module_functions;
}


// Get server core functions
const ABI_SERVER_CORE_FUNCTIONS *sdk_imodule_get_server_core_functions(const IModule *module) {
    return &module->server_functions;
}


// ================================== REGISTERERS ==================================

// Register destroy
void sdk_module_register_destroy(IModule *module, void (*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_destroy = callback;
}

// Get configuration
void sdk_module_register_get_configuration(IModule *module, const char *(*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_get_configuration = callback;
}

// Get data
void sdk_module_register_get_data(IModule *module, ABI_MODULE_MDTP_DATA *(*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_get_data = callback;
}

// Enable module
void sdk_module_register_enable(IModule *module, void (*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_enable = callback;
}

// Disable module
void sdk_module_register_disable(IModule *module, void (*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_disable = callback;
}

// Is module enabled?
void sdk_module_register_is_enabled(IModule *module, uint8_t (*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_is_enabled = callback;
}

// Get module name
void sdk_module_register_get_module_name(IModule *module, const char *(*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_get_module_name = callback;
}

// Get module description
void sdk_module_register_get_module_description(IModule *module, const char *(*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_get_module_description = callback;
}

// Set poll ratio of module
void sdk_module_register_set_poll_ratio(IModule *module, void (*callback)(uint32_t poll_ratio)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_set_poll_ratio = callback;
}

// Get poll ratio of module
void sdk_module_register_get_poll_ratio(IModule *module, uint32_t (*callback)(void)) {
    if (!module || !callback) {
        return;
    }

    module->module_functions.module_get_poll_ratio = callback;
}

#ifdef __cplusplus
}
#endif
