#include <modules/sdk.h>
#include <unity.h>


ABI_MODULE_FUNCTIONS module_init(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                 const char               *json_configuration) {
    return (ABI_MODULE_FUNCTIONS){0};
}


uint32_t server_abi_version_stub(const ABI_MODULE_CONTEXT *context) {
    return 1;
}


void server_abi_log(const ABI_MODULE_CONTEXT *context, int log_type, const char* msg) {
    TEST_ASSERT_EQUAL_STRING("name", context->module_name);
    TEST_ASSERT_EQUAL_STRING("description", context->module_description);

    TEST_ASSERT_EQUAL(LOG_INFO, log_type);
    TEST_ASSERT_EQUAL_STRING("test log", msg);
}


void tearDown(void) {
}

void setUp(void) {}


IModule* module;


// ================================== TESTS ==================================

void test_get_abi_version(void) {
    TEST_ASSERT_EQUAL(1, sdk_utils_get_server_abi_version(module));
}


void test_log(void) {
    sdk_utils_log(module, LOG_INFO, "test log");
}

// ================================== MAIN ==================================

int main(void) {
    ABI_SERVER_CORE_FUNCTIONS server_functions = (ABI_SERVER_CORE_FUNCTIONS){
        .abi_get_abi_version = server_abi_version_stub, .abi_log = server_abi_log};

    // Call module init
    module = sdk_imodule_create("name", "description", server_functions, 0, 0);

    UNITY_BEGIN();

    RUN_TEST(test_get_abi_version);
    RUN_TEST(test_log);

    sdk_imodule_destroy(module);

    return UNITY_END();
}
