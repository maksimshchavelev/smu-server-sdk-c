#include "modules/internals/memutils.h"
#include "modules/internals/sdk_internals.h"
#include "modules/sdk.h"
#include <unity.h>


// =================== STUBS ====================

SDKStatus sdk_module_init(const char *json_config) {
    return SDK_OK;
}


void sdk_module_destroy(void) {}


const char *sdk_module_get_configuration(void) {
    return 0;
}


ABI_MODULE_MDTP_DATA *sdk_module_get_data(void) {
    return 0;
}


void sdk_module_enable(void) {}



void sdk_module_disable(void) {}


uint8_t sdk_module_is_enabled(void) {
    return 1;
}


// ================ END OF STUBS ================

SDK_UTILS *utils;


int server_abi_version_stub(ABI_MODULE_CONTEXT *context) {
    return 1;
}


void server_abi_log(ABI_MODULE_CONTEXT *context, int log_type, const char* msg) {
    TEST_ASSERT_EQUAL_STRING("name", context->module_name);
    TEST_ASSERT_EQUAL_STRING("description", context->module_description);

    TEST_ASSERT_EQUAL(LOG_INFO, log_type);

    TEST_ASSERT_EQUAL_STRING("test log", msg);
}


void tearDown(void) {
}

void setUp(void) {}


// ================================== TESTS ==================================

void test_module_setup(void) {
    utils->module_setup("name", "description");
}


void test_get_module_name(void) {
    TEST_ASSERT_EQUAL_STRING("name", utils->get_module_name());
}


void test_get_module_description(void) {
    TEST_ASSERT_EQUAL_STRING("description", utils->get_module_description());
}


void test_get_module_context(void) {
    ABI_MODULE_CONTEXT *ctx = utils->get_module_context();

    TEST_ASSERT_EQUAL_STRING("name", ctx->module_name);
    TEST_ASSERT_EQUAL_STRING("description", ctx->module_description);
}


void test_log(void) {
    utils->log(LOG_INFO, "test log");
}

// ================================== MAIN ==================================

int main(void) {
    ABI_SERVER_CORE_FUNCTIONS server_functions = (ABI_SERVER_CORE_FUNCTIONS){
        .abi_get_abi_version = server_abi_version_stub, .abi_log = server_abi_log};

    // Call module init
    module_init(server_functions, "");
    utils = sdk_utils_get();

    UNITY_BEGIN();

    RUN_TEST(test_module_setup);
    RUN_TEST(test_get_module_name);
    RUN_TEST(test_get_module_description);
    RUN_TEST(test_get_module_context);
    RUN_TEST(test_log);

    module_destroy();

    return UNITY_END();
}
