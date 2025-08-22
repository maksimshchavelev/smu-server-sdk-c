#include <modules/sdk.h>


SDKStatus sdk_module_init(const char *json_config) {
    return SDK_OK;
}


void sdk_module_destroy(void) {}


const char *sdk_module_get_configuration(void) {
    return 0;
}


SDK_MODULE_MDTP_DATA *sdk_module_get_data(void) {
    return 0;
}


void sdk_module_enable(void) {}



void sdk_module_disable(void) {}


uint8_t sdk_module_is_enabled(void) {
    return 1;
}
