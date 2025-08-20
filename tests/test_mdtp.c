#include <internals/sdk_internals.h>
#include <internals/memutils.h>
#include <unity.h>


void setUp(void) {}

void tearDown(void) {}


void test_make_value_node(void) {
    void *node = sdk_mdtp_make_value("RAM", "1234", "MB");
    TEST_ASSERT_NOT_NULL(node);

    TEST_ASSERT_EQUAL(((uint32_t *)node)[0], 1); // 1 is value
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


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_make_value_node);
    return UNITY_END();
}
