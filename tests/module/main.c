#include "modules/internals/sdk_internals.h"
#include "modules/sdk.h"
#include <unity.h>


void tearDown(void) {}

void setUp(void) {}


// Constants
static const char *module_name = "MODULE";
static const char *module_description = "Test module";

static const int abi_version = 1;

static const char   *test_log = "This is test log message";
static const LogType test_log_type = LOG_WARNING;

static const char *test_json_configuration = "{"
                                             "\"field\": 5"
                                             "}";
// End of constants

ABI_MODULE_FUNCTIONS *module_functions;

// ================================ FUNCTIONS DONE ================================

/**
 * @brief Structure for tracking whether any functions are called at all
 */
typedef struct FUNCTIONS_DONE {
    // SDK_UTILS get
    unsigned test_sdk_utils_get : 1;
    unsigned test_sdk_utils_mdtp_create_and_destroy_value_node : 1;
    // ABI
    unsigned test_abi_get_abi_version : 1;
    unsigned test_abi_log : 1;
    // SDK
    unsigned test_sdk_module_init : 1;
    unsigned test_sdk_module_destroy : 1;
    unsigned test_sdk_module_get_configuration : 1;
    unsigned test_sdk_module_get_data : 1;
    unsigned test_sdk_module_enable : 1;
    unsigned test_sdk_module_disable : 1;
    unsigned test_sdk_module_is_enabled : 1;
} FUNCTIONS_DONE;

static FUNCTIONS_DONE functions_done = {0};

// =============================================================================


// Server functions stubs
int test_module_functions_abi_get_abi_version(ABI_MODULE_CONTEXT *ctx) {
    // Anonymously module yet
    TEST_ASSERT_EQUAL_STRING("anonymously", ctx->module_name);
    TEST_ASSERT_EQUAL_STRING("anonymously", ctx->module_description);
    functions_done.test_abi_get_abi_version = 1;
    return abi_version;
}

void test_module_functions_abi_log(ABI_MODULE_CONTEXT *ctx, int log_type, const char *msg) {
    TEST_ASSERT_EQUAL_STRING(module_name, ctx->module_name);
    TEST_ASSERT_EQUAL_STRING(module_description, ctx->module_description);
    TEST_ASSERT_EQUAL_STRING(test_log, msg);
    TEST_ASSERT_EQUAL((int)test_log_type, log_type);
}

// ================================= TESTS ============================================

// ABI module init
void test_module_init(void) {
    ABI_SERVER_CORE_FUNCTIONS server_functions = (ABI_SERVER_CORE_FUNCTIONS){
        .abi_get_abi_version = test_module_functions_abi_get_abi_version,
        .abi_log = test_module_functions_abi_log};

    // Call module init
    module_functions = module_init(server_functions, test_json_configuration);
    TEST_ASSERT_TRUE(module_functions != NULL);
}


// ABI module destroy
void test_module_destroy(void) {
    module_functions->module_destroy();
}


// Check get data
void test_module_get_data(void) {
    ABI_MODULE_MDTP_DATA *data = module_functions->module_get_data();
    TEST_ASSERT_TRUE(data != NULL);
    TEST_ASSERT_TRUE(data->data != NULL);
    TEST_ASSERT_TRUE(data->size > 0);
}


// Check get configuration
void test_module_get_configuration(void) {
    const char* str = module_functions->module_get_configuration();
    TEST_ASSERT_TRUE(str != NULL);
    TEST_ASSERT_EQUAL_STRING(test_json_configuration, str);
}


// Check enable
void test_module_enable(void) {
    module_functions->module_enable();
}


// Check disable
void test_module_disable(void) {
    module_functions->module_disable();
}


// Check enabled
void test_module_is_enabled(void) {
    module_functions->module_is_enabled();
}



// Check which module functions were called during tests
void test_functions_done(void) {
    TEST_ASSERT_EQUAL(1, functions_done.test_abi_get_abi_version);
    TEST_ASSERT_EQUAL(1, functions_done.test_abi_get_abi_version);

    TEST_ASSERT_EQUAL(1, functions_done.test_sdk_module_init);
    TEST_ASSERT_EQUAL(1, functions_done.test_sdk_module_destroy);
    TEST_ASSERT_EQUAL(1, functions_done.test_sdk_module_get_data);
    TEST_ASSERT_EQUAL(1, functions_done.test_sdk_module_get_configuration);
    TEST_ASSERT_EQUAL(1, functions_done.test_sdk_module_enable);
    TEST_ASSERT_EQUAL(1, functions_done.test_sdk_module_disable);
    TEST_ASSERT_EQUAL(1, functions_done.test_sdk_module_is_enabled);
}


// ======================== IMPLEMENTED MODULE FUNCTIONS ========================


SDKStatus sdk_module_init(const char *json_config) {
    TEST_ASSERT_EQUAL(SDK_OK, sdk_utils_get()->module_setup(module_name, module_description));
    TEST_ASSERT_EQUAL_STRING(test_json_configuration, json_config);
    functions_done.test_sdk_module_init = 1;
    return SDK_OK;
}


void sdk_module_destroy(void) {
    functions_done.test_sdk_module_destroy = 1;
}


const char *sdk_module_get_configuration(void) {
    functions_done.test_sdk_module_get_configuration = 1;
    return test_json_configuration;
}


ABI_MODULE_MDTP_DATA *sdk_module_get_data(void) {
    MDTP_UTILS mdtp = sdk_utils_get()->mdtp;
    functions_done.test_sdk_module_get_data = 1;
    return mdtp.make_root(mdtp.make_value("ram used", "55", "MB"), NULL);
}


void sdk_module_enable(void) {
    functions_done.test_sdk_module_enable = 1;
}



void sdk_module_disable(void) {
    functions_done.test_sdk_module_disable = 1;
}


uint8_t sdk_module_is_enabled(void) {
    functions_done.test_sdk_module_is_enabled = 1;
    return 1;
}


// ======================== MAIN ========================

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_module_init);
    RUN_TEST(test_module_destroy);
    RUN_TEST(test_module_get_data);
    RUN_TEST(test_module_get_configuration);
    RUN_TEST(test_module_enable);
    RUN_TEST(test_module_disable);
    RUN_TEST(test_module_is_enabled);

    RUN_TEST(test_functions_done);

    return UNITY_END();
}
