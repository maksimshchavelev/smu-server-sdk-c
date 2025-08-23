#include <unity.h>
#include "test_mdtp.h"
#include "test_sdk_stubs.h"


int main(void) {
    UNITY_BEGIN();

    test_run_mdtp();

    return UNITY_END();
}
