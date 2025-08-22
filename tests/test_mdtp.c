#include "modules/sdk.h"
#include "modules/internals/memutils.h"
#include "modules/internals/sdk_internals.h"
#include <unity.h>


void setUp(void) {}

void tearDown(void) {}


int server_abi_version_stub(SDK_MODULE_ABI_CONTEXT *context) {
    return 1;
}


void test_make_value_node(void) {
    void *node = sdk_mdtp_make_value("RAM", "1234", "MB");
    TEST_ASSERT_NOT_NULL(node);

    TEST_ASSERT_EQUAL(((uint32_t *)node)[0], 1);   // 1 is value
    TEST_ASSERT_EQUAL(read_uint32_be(node, 1), 3); // node name length

    // Node name
    TEST_ASSERT_EQUAL(((char *)node)[5], 'R');
    TEST_ASSERT_EQUAL(((char *)node)[6], 'A');
    TEST_ASSERT_EQUAL(((char *)node)[7], 'M');

    TEST_ASSERT_EQUAL(read_uint32_be(node, 8), 2); // Units length

    // Units
    TEST_ASSERT_EQUAL(((char *)node)[12], 'M');
    TEST_ASSERT_EQUAL(((char *)node)[13], 'B');

    TEST_ASSERT_EQUAL(read_uint32_be(node, 14), 4); // Value length

    // Value
    TEST_ASSERT_EQUAL(((char *)node)[18], '1');
    TEST_ASSERT_EQUAL(((char *)node)[19], '2');
    TEST_ASSERT_EQUAL(((char *)node)[20], '3');
    TEST_ASSERT_EQUAL(((char *)node)[21], '4');

    sdk_mdtp_free_value(node);
}



void test_make_empty_value_node(void) {
    void *node = sdk_mdtp_make_value("", "", "");
    TEST_ASSERT_NOT_NULL(node);

    TEST_ASSERT_EQUAL(((uint32_t *)node)[0], 1);   // 1 is value
    TEST_ASSERT_EQUAL(read_uint32_be(node, 1), 0); // node name length
    TEST_ASSERT_EQUAL(read_uint32_be(node, 5), 0); // Units length
    TEST_ASSERT_EQUAL(read_uint32_be(node, 9), 0); // Value length

    sdk_mdtp_free_value(node);
}



void test_make_container_node(void) {
    void *node = sdk_mdtp_make_container("ram", sdk_mdtp_make_value("use", "12", "gb"), NULL);

    TEST_ASSERT_NOT_NULL(node);

    TEST_ASSERT_EQUAL(((uint32_t *)node)[0], 0);   // 0 is container
    TEST_ASSERT_EQUAL(read_uint32_be(node, 1), 3); // node name length (ram)

    // Node name
    TEST_ASSERT_EQUAL(((char *)node)[5], 'r');
    TEST_ASSERT_EQUAL(((char *)node)[6], 'a');
    TEST_ASSERT_EQUAL(((char *)node)[7], 'm');

    TEST_ASSERT_EQUAL(read_uint32_be(node, 8), 20); // Payload size

    // Payload
    TEST_ASSERT_EQUAL(read_ubyte_be(node, 12), 1);  // node type
    TEST_ASSERT_EQUAL(read_uint32_be(node, 13), 3); // node name length

    // Node name
    TEST_ASSERT_EQUAL(((char *)node)[17], 'u');
    TEST_ASSERT_EQUAL(((char *)node)[18], 's');
    TEST_ASSERT_EQUAL(((char *)node)[19], 'e');

    TEST_ASSERT_EQUAL(read_uint32_be(node, 20), 2); // units length

    // Units
    TEST_ASSERT_EQUAL(((char *)node)[24], 'g');
    TEST_ASSERT_EQUAL(((char *)node)[25], 'b');

    TEST_ASSERT_EQUAL(read_uint32_be(node, 26), 2); // value length

    // Value
    TEST_ASSERT_EQUAL(((char *)node)[30], '1');
    TEST_ASSERT_EQUAL(((char *)node)[31], '2');

    sdk_mdtp_free_value(node);
}



void test_make_root_node(void) {
    module_init((SDK_ABI_SERVER_CORE_FUNCTIONS){server_abi_version_stub, NULL}, "");

    SDK_MODULE_MDTP_DATA *data = sdk_mdtp_make_root(
        sdk_mdtp_make_container("ram", sdk_mdtp_make_value("use", "12", "gb"), NULL), NULL);

    TEST_ASSERT_NOT_NULL(data);

    TEST_ASSERT_EQUAL(data->size, 37);

    TEST_ASSERT_EQUAL(((uint32_t *)data->data)[0], MDTP_VERSION); // MDTP version
    TEST_ASSERT_EQUAL(read_uint32_be((void *)data->data, 1), 32); // Payload size

    TEST_ASSERT_EQUAL(read_ubyte_be((void *)data->data, 5), 0);           // 0 is container
    TEST_ASSERT_EQUAL(read_uint32_be((void *)data->data, 6), 3); // node name length (ram)

    // Node name
    TEST_ASSERT_EQUAL(((char *)data->data)[10], 'r');
    TEST_ASSERT_EQUAL(((char *)data->data)[11], 'a');
    TEST_ASSERT_EQUAL(((char *)data->data)[12], 'm');

    TEST_ASSERT_EQUAL(read_uint32_be((void *)data->data, 13), 20); // Payload size

    // Payload
    TEST_ASSERT_EQUAL(read_ubyte_be((void *)data->data, 17), 1);  // node type
    TEST_ASSERT_EQUAL(read_uint32_be((void *)data->data, 18), 3); // node name length

    // Node name
    TEST_ASSERT_EQUAL(((char *)data->data)[22], 'u');
    TEST_ASSERT_EQUAL(((char *)data->data)[23], 's');
    TEST_ASSERT_EQUAL(((char *)data->data)[24], 'e');

    TEST_ASSERT_EQUAL(read_uint32_be((void *)data->data, 25), 2); // units length

    // Units
    TEST_ASSERT_EQUAL(((char *)data->data)[29], 'g');
    TEST_ASSERT_EQUAL(((char *)data->data)[30], 'b');

    TEST_ASSERT_EQUAL(read_uint32_be((void *)data->data, 31), 2); // value length

    // Value
    TEST_ASSERT_EQUAL(((char *)data->data)[35], '1');
    TEST_ASSERT_EQUAL(((char *)data->data)[36], '2');

    module_destroy();
}




int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_make_value_node);
    RUN_TEST(test_make_empty_value_node);
    RUN_TEST(test_make_container_node);
    RUN_TEST(test_make_root_node);

    return UNITY_END();
}
