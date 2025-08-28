/**
 * @file main.c
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, your name <your email>
 */

#include <smu-server/sdk/c/modules/sdk.h> // For C SDK
#include <stddef.h>                       // For NULL

// =====================================================================
// Forward declaration of functions that must be implemented in a module
// You can give these functions any names (that do not overlap with sdk_* names). 
// This is your module. Do whatever you want here. You implement these functions 
// yourself and then register them (see below).


/**
 * @brief The function destroys the module. At the end it must destroy `module`
 */
void destroy(void);

/**
 * @brief Here you need to correctly respond to the request to enable the module and enable it
 */
void enable(void);

/**
 * @brief Here you need to correctly respond to the request to turn off the module and turn it off
 */
void disable(void);

/**
 * @brief Here we need to return information about the module state
 * @return `1` if module is enabled, otherwise `0`
 */
uint8_t is_enabled(void);

/**
 * @brief Here we need to return the current module configuration
 * @return NULL-terminated string with Json configuration
 */
const char *get_configuration(void);

/**
 * @brief Here we have to generate data using MDTP protocol and functions like `sdk_mdtp_*` and
 * return it
 * @return `ABI_MODULE_MDTP_DATA *`
 */
const ABI_MODULE_MDTP_DATA *get_data(void);

/**
 * @brief Here we need to return the module name
 * @return NULL-terminated string with module name
 */
const char *get_name(void);

/**
 * @brief Here we need to return the module description
 * @return NULL-terminated string with module description
 */
const char *get_description(void);

/**
 * @brief Here you need to return the poll ratio for the module
 * @return Poll ratio
 */
uint32_t get_poll_ratio(void);

/**
 * @brief Here you need to set the poll ratio
 * @param poll_ratio Required poll ratio
 */
void set_poll_ratio(uint32_t poll_ratio);



// End of forward declaration
// =====================================================================


/**
 * @brief Declare a pointer to IModule.
 *
 * IModule is a structure that stores the state and parameters of our module. We cannot interact
 * with its fields directly, but we can do this through the `sdk_imodule_*` functions.
 */
static IModule *module = NULL;

/**
 * @brief This is the entry point to the module.
 *
 * Here we initialize the module and return a filled structure with pointers to the module functions
 * so that the server can interact with our module.
 *
 * @param server_functions A structure with pointers to server functions so we can access them via
 * `sdk_utils_*` wrappers
 * @param json_configuration Json module configuration. We can use Parson to work with Json by
 * simply including
 * @return `ABI_MODULE_FUNCTIONS`
 */
const ABI_MODULE_FUNCTIONS *module_init(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                        const char               *json_configuration) {
    // We allocate memory for IModule and initialize it
    //
    // Note that the module name must match the name of the target you are creating. Case matters!
    // Also note that to free memory at the end of the module (in the destroy function) you need to
    // call `sdk_imodule_destroy`. This will also free the internal resources of the module. You
    // cannot simply call `free((void *)module)`
    //
    // We initialize `poll_ratio` and `is_enabled` to `1`. In real modules you should read these
    // parameters from the Json configuration.
    module = sdk_imodule_create("MyWonderfulModule", "Example module", server_functions, 1, 1);

    // Return NULL if sdk_imodule_create failed
    if (!module) {
        return NULL;
    }

    // Here we register handlers which the server calls
    sdk_module_register_destroy(module, destroy);
    sdk_module_register_enable(module, enable);
    sdk_module_register_disable(module, disable);
    sdk_module_register_is_enabled(module, is_enabled);
    sdk_module_register_get_configuration(module, get_configuration);
    sdk_module_register_get_data(module, get_data);
    sdk_module_register_get_module_name(module, get_name);
    sdk_module_register_get_module_description(module, get_description);
    sdk_module_register_get_poll_ratio(module, get_poll_ratio);
    sdk_module_register_set_poll_ratio(module, set_poll_ratio);

    // Then we can do some of our own work. But we just log the greeting
    sdk_utils_log(module, LOG_INFO, "Greetings from a module written in C!");

    // Return function table
    return sdk_imodule_get_module_functions(module);
}


// =============================== IMPLEMENTATION OF FUNCTIONS ===============================

void destroy(void) {
    // Do something, like free up your resources. We log the completion of work
    sdk_utils_log(module, LOG_INFO, "My wonderful module is being destroyed. Bye-bye :)");

    // Finally, destroy the `module`
    sdk_imodule_destroy(module);
}


void enable(void) {
    sdk_imodule_enable(module);
}


void disable(void) {
    sdk_imodule_disable(module);
}


uint8_t is_enabled(void) {
    return sdk_imodule_is_enabled(module);
}


const char *get_configuration(void) {
    // Return empty configuration
    // In real form you should save the configuration either as Json (more convenient if you change
    // it) or as a string and return the saved configuration
    return "{"
           "\"enabled\": true"
           "}";
}


const ABI_MODULE_MDTP_DATA *get_data(void) {
    // In the real module you should create your meaningful data. Now we return the stub
    return sdk_mdtp_make_root(
        module,
        sdk_mdtp_make_container(
            "Traffic", sdk_mdtp_make_value("Traffic transferred", "4567.4", "GB"), NULL),
        NULL);
}


const char *get_name(void) {
    // We get the module context and extract its name from it.
    return sdk_imodule_get_context(module)->module_name;
}


const char *get_description(void) {
    // We get the module context and extract its description from it.
    return sdk_imodule_get_context(module)->module_description;
}


uint32_t get_poll_ratio(void) {
    return sdk_imodule_get_poll_ratio(module);
}


void set_poll_ratio(uint32_t poll_ratio) {
    sdk_imodule_set_poll_ratio(module, poll_ratio);
}
