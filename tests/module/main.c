#include <modules/sdk.h>
#include <string.h>
#include <unity.h>

/* --------------- stubs ---------------- */

ABI_MODULE_FUNCTIONS module_init(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                 const char               *json_configuration) {
    return (ABI_MODULE_FUNCTIONS){0};
}

/* ---------- Test-wide globals ---------- */

/** @brief Global module pointer used by callbacks (ABI callbacks have no args). */
static IModule *g_module = NULL;

/** @brief Cached pointer to module's ABI function table. */
static const ABI_MODULE_FUNCTIONS *g_abi = NULL;

/** @brief Bookkeeping counters to assert call frequencies. */
static struct
{
    int destroy_calls;
    int get_conf_calls;
    int get_data_calls;
    int enable_calls;
    int disable_calls;
    int is_enabled_calls;
    int get_name_calls;
    int get_desc_calls;
    int set_pr_calls;
    int get_pr_calls;

    int srv_log_calls;
    int srv_ver_calls;
} g_calls;

/** @brief Last-observed server log parameters. */
static struct
{
    const ABI_MODULE_CONTEXT *ctx;
    int                       log_type;
    const char               *msg;
} g_last_log;

/** @brief Last-observed server version request context. */
static const ABI_MODULE_CONTEXT *g_last_ver_ctx = NULL;

/* ---------- Fake server core (emulated by tests) ---------- */

/**
 * @brief Fake server: return ABI version; record context usage.
 */
static uint32_t fake_abi_get_abi_version(const ABI_MODULE_CONTEXT *context)
{
    g_calls.srv_ver_calls++;
    g_last_ver_ctx = context;
    /* Any test value is fine; we assert it round-trips. */
    return 0xBEEFCAFEu;
}

/**
 * @brief Fake server: log; record parameters.
 */
static void fake_abi_log(const ABI_MODULE_CONTEXT *context, int log_type, const char *message)
{
    g_calls.srv_log_calls++;
    g_last_log.ctx      = context;
    g_last_log.log_type = log_type;
    g_last_log.msg      = message;
}

/** @brief Pre-baked server vtable for sdk_imodule_create. */
static ABI_SERVER_CORE_FUNCTIONS g_server = {
    .abi_get_abi_version = fake_abi_get_abi_version,
    .abi_log             = fake_abi_log,
};

/* ---------- Stub module callbacks (what a real .so would register) ---------- */

/**
 * @brief Return module JSON configuration.
 * @return Static JSON string.
 */
static const char *stub_get_configuration(void)
{
    g_calls.get_conf_calls++;
    return "{\"demo\":true,\"poll\":42}";
}

/**
 * @brief Produce MDTP data using MDTP utils into the module's storage.
 * @return Non-NULL pointer with size > 0 (asserted in tests).
 */
static const ABI_MODULE_MDTP_DATA *stub_get_data(void)
{
    g_calls.get_data_calls++;

    /* Create simplest valid MDTP tree: root(container(value)). Ownership is transferred. */
    void *container = sdk_mdtp_make_container(
        "metrics",
        sdk_mdtp_make_value("foo", "1", "u"),
        NULL);

    return sdk_mdtp_make_root(g_module, container, NULL);
}

/**
 * @brief Enable: flip SDK state.
 */
static void stub_enable(void)
{
    g_calls.enable_calls++;
    sdk_imodule_enable(g_module);
}

/**
 * @brief Disable: flip SDK state.
 */
static void stub_disable(void)
{
    g_calls.disable_calls++;
    sdk_imodule_disable(g_module);
}

/**
 * @brief Report enable flag via SDK.
 */
static uint8_t stub_is_enabled(void)
{
    g_calls.is_enabled_calls++;
    return sdk_imodule_is_enabled(g_module);
}

/**
 * @brief Return module name from the SDK context.
 */
static const char *stub_get_module_name(void)
{
    g_calls.get_name_calls++;
    const ABI_MODULE_CONTEXT *ctx = sdk_imodule_get_context(g_module);
    return ctx ? ctx->module_name : NULL;
}

/**
 * @brief Return module description from the SDK context.
 */
static const char *stub_get_module_description(void)
{
    g_calls.get_desc_calls++;
    const ABI_MODULE_CONTEXT *ctx = sdk_imodule_get_context(g_module);
    return ctx ? ctx->module_description : NULL;
}

/**
 * @brief Set poll ratio in SDK.
 */
static void stub_set_poll_ratio(uint32_t pr)
{
    g_calls.set_pr_calls++;
    sdk_imodule_set_poll_ratio(g_module, pr);
}

/**
 * @brief Get poll ratio from SDK.
 */
static uint32_t stub_get_poll_ratio(void)
{
    g_calls.get_pr_calls++;
    return sdk_imodule_get_poll_ratio(g_module);
}

/**
 * @brief Destroy callback – in production should free resources.
 *        Here we only count the call; real destruction is done at end of main().
 */
static void stub_destroy(void)
{
    g_calls.destroy_calls++;
    /* Intentionally DO NOT call sdk_imodule_destroy() here for test isolation.
       We destroy once at the very end of main(). */
}

/* ---------- Unity fixtures ---------- */

void setUp(void)
{
    /* Reset only counters that should be fresh per-test. */
    memset(&g_calls, 0, sizeof(g_calls));
    memset(&g_last_log, 0, sizeof(g_last_log));
    g_last_ver_ctx = NULL;
}

void tearDown(void)
{
    /* Nothing */
}

/* ---------- Helper assertions ---------- */

/**
 * @brief Assert that ABI vtable points to our stub callbacks.
 */
static void assert_callbacks_are_wired(void)
{
    TEST_ASSERT_NOT_NULL(g_abi);

    TEST_ASSERT_EQUAL_PTR(stub_get_configuration,      g_abi->module_get_configuration);
    TEST_ASSERT_EQUAL_PTR(stub_get_data,               g_abi->module_get_data);
    TEST_ASSERT_EQUAL_PTR(stub_enable,                 g_abi->module_enable);
    TEST_ASSERT_EQUAL_PTR(stub_disable,                g_abi->module_disable);
    TEST_ASSERT_EQUAL_PTR(stub_is_enabled,             g_abi->module_is_enabled);
    TEST_ASSERT_EQUAL_PTR(stub_get_module_name,        g_abi->module_get_module_name);
    TEST_ASSERT_EQUAL_PTR(stub_get_module_description, g_abi->module_get_module_description);
    TEST_ASSERT_EQUAL_PTR(stub_set_poll_ratio,         g_abi->module_set_poll_ratio);
    TEST_ASSERT_EQUAL_PTR(stub_get_poll_ratio,         g_abi->module_get_poll_ratio);

    /* Destroy pointer presence (we won't invoke it until a special test). */
    TEST_ASSERT_EQUAL_PTR(stub_destroy,                g_abi->module_destroy);
}

/* ---------- Tests ---------- */

/**
 * @brief Module must expose server core functions passed at creation.
 */
static void test_server_core_is_exposed(void)
{
    const ABI_SERVER_CORE_FUNCTIONS *sc = sdk_imodule_get_server_core_functions(g_module);
    TEST_ASSERT_NOT_NULL(sc);
    TEST_ASSERT_EQUAL_PTR(fake_abi_get_abi_version, sc->abi_get_abi_version);
    TEST_ASSERT_EQUAL_PTR(fake_abi_log,             sc->abi_log);
}

/**
 * @brief Registration must wire all callbacks into ABI table.
 */
static void test_registration_wires_callbacks(void)
{
    assert_callbacks_are_wired();
}

/**
 * @brief Getting configuration returns expected JSON and bumps counter.
 */
static void test_get_configuration(void)
{
    const char *js = g_abi->module_get_configuration();
    TEST_ASSERT_NOT_NULL(js);
    TEST_ASSERT_EQUAL_STRING("{\"demo\":true,\"poll\":42}", js);
    TEST_ASSERT_EQUAL_INT(1, g_calls.get_conf_calls);
}

/**
 * @brief MDTP data callback must return non-NULL and size > 0.
 */
static void test_get_mdtp_data_nonnull_and_sized(void)
{
    const ABI_MODULE_MDTP_DATA *d = g_abi->module_get_data();
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_NOT_NULL(d->data);
    TEST_ASSERT_TRUE(d->size > 0u);
    TEST_ASSERT_EQUAL_INT(1, g_calls.get_data_calls);
}

/**
 * @brief Enable/disable flow flips the state visible via module_is_enabled().
 */
static void test_enable_disable_flow(void)
{
    /* Ensure known state: disable then enable */
    g_abi->module_disable();
    TEST_ASSERT_EQUAL_UINT8(0u, g_abi->module_is_enabled());
    TEST_ASSERT_TRUE(g_calls.disable_calls >= 1);
    TEST_ASSERT_TRUE(g_calls.is_enabled_calls >= 1);

    g_abi->module_enable();
    TEST_ASSERT_EQUAL_UINT8(1u, g_abi->module_is_enabled());
}

/**
 * @brief Poll ratio set/get round-trips through SDK.
 */
static void test_poll_ratio_roundtrip(void)
{
    const uint32_t want = 777u;
    g_abi->module_set_poll_ratio(want);
    uint32_t got = g_abi->module_get_poll_ratio();
    TEST_ASSERT_EQUAL_UINT32(want, got);
    TEST_ASSERT_EQUAL_INT(1, g_calls.set_pr_calls);
    TEST_ASSERT_EQUAL_INT(1, g_calls.get_pr_calls);
}

/**
 * @brief Name/description come from SDK context and match creation args.
 */
static void test_name_and_description(void)
{
    const char *name = g_abi->module_get_module_name();
    const char *desc = g_abi->module_get_module_description();
    TEST_ASSERT_EQUAL_STRING("TestMod", name);
    TEST_ASSERT_EQUAL_STRING("Module used in Unity tests", desc);
}

/**
 * @brief Server-utils wrappers should call through to server core vtable.
 */
static void test_server_utils_wrappers(void)
{
    /* Version */
    uint32_t ver = sdk_utils_get_server_abi_version(g_module);
    TEST_ASSERT_EQUAL_UINT32(0xBEEFCAFEu, ver);
    TEST_ASSERT_EQUAL_INT(1, g_calls.srv_ver_calls);

    /* The server receives the exact context of our module */
    const ABI_MODULE_CONTEXT *ctx = sdk_imodule_get_context(g_module);
    TEST_ASSERT_EQUAL_PTR(ctx, g_last_ver_ctx);

    /* Logging */
    sdk_utils_log(g_module, (LogType)1, "hello");
    TEST_ASSERT_EQUAL_INT(1, g_calls.srv_log_calls);
    TEST_ASSERT_EQUAL_PTR(ctx, g_last_log.ctx);
    TEST_ASSERT_EQUAL_INT(1, g_last_log.log_type);
    TEST_ASSERT_EQUAL_STRING("hello", g_last_log.msg);
}

/**
 * @brief Registering with NULL must have no effect (per contract).
 */
static void test_null_registration_has_no_effect(void)
{
    /* Snapshot current pointers */
    const ABI_MODULE_FUNCTIONS *before = g_abi;

    /* NULL module -> no effect */
    sdk_module_register_get_configuration(NULL, stub_get_configuration);
    sdk_module_register_get_data(NULL, stub_get_data);
    sdk_module_register_enable(NULL, stub_enable);
    sdk_module_register_disable(NULL, stub_disable);
    sdk_module_register_is_enabled(NULL, stub_is_enabled);
    sdk_module_register_get_module_name(NULL, stub_get_module_name);
    sdk_module_register_get_module_description(NULL, stub_get_module_description);
    sdk_module_register_set_poll_ratio(NULL, stub_set_poll_ratio);
    sdk_module_register_get_poll_ratio(NULL, stub_get_poll_ratio);
    sdk_module_register_destroy(NULL, stub_destroy);

    /* NULL callback -> no effect */
    sdk_module_register_get_configuration(g_module, NULL);
    sdk_module_register_get_data(g_module, NULL);
    sdk_module_register_enable(g_module, NULL);
    sdk_module_register_disable(g_module, NULL);
    sdk_module_register_is_enabled(g_module, NULL);
    sdk_module_register_get_module_name(g_module, NULL);
    sdk_module_register_get_module_description(g_module, NULL);
    sdk_module_register_set_poll_ratio(g_module, NULL);
    sdk_module_register_get_poll_ratio(g_module, NULL);
    sdk_module_register_destroy(g_module, NULL);

    /* Pointers must remain the same */
    assert_callbacks_are_wired();
    TEST_ASSERT_EQUAL_PTR(before, g_abi);
}

/**
 * @brief Destroy callback is present and callable; we don't free inside it.
 *
 * We only check it increments the counter. The actual memory is freed once in main().
 */
static void test_destroy_callback_is_invokable(void)
{
    TEST_ASSERT_EQUAL_INT(0, g_calls.destroy_calls);
    g_abi->module_destroy();
    TEST_ASSERT_EQUAL_INT(1, g_calls.destroy_calls);
}

/* ---------- Unity runner with required create/destroy ---------- */

int main(void)
{
    UNITY_BEGIN();

    /* Create a single IModule instance for all tests, as requested. */
    g_module = sdk_imodule_create(
        "TestMod",
        "Module used in Unity tests",
        g_server,
        10u,   /* initial poll ratio */
        1u     /* initially enabled   */
        );
    TEST_ASSERT_NOT_NULL(g_module);

    /* Wire all callbacks for this module instance. */
    sdk_module_register_get_configuration(g_module,      stub_get_configuration);
    sdk_module_register_get_data(g_module,               stub_get_data);
    sdk_module_register_enable(g_module,                 stub_enable);
    sdk_module_register_disable(g_module,                stub_disable);
    sdk_module_register_is_enabled(g_module,             stub_is_enabled);
    sdk_module_register_get_module_name(g_module,        stub_get_module_name);
    sdk_module_register_get_module_description(g_module, stub_get_module_description);
    sdk_module_register_set_poll_ratio(g_module,         stub_set_poll_ratio);
    sdk_module_register_get_poll_ratio(g_module,         stub_get_poll_ratio);
    sdk_module_register_destroy(g_module,                stub_destroy);

    /* Cache ABI vtable for convenience. */
    g_abi = sdk_imodule_get_module_functions(g_module);
    TEST_ASSERT_NOT_NULL(g_abi);

    /* Run tests */
    RUN_TEST(test_server_core_is_exposed);
    RUN_TEST(test_registration_wires_callbacks);
    RUN_TEST(test_get_configuration);
    RUN_TEST(test_get_mdtp_data_nonnull_and_sized);
    RUN_TEST(test_enable_disable_flow);
    RUN_TEST(test_poll_ratio_roundtrip);
    RUN_TEST(test_name_and_description);
    RUN_TEST(test_server_utils_wrappers);
    RUN_TEST(test_null_registration_has_no_effect);
    RUN_TEST(test_destroy_callback_is_invokable);

    /* Mandatory: explicitly destroy the module at the very end. */
    sdk_imodule_destroy(g_module);
    g_module = NULL;

    return UNITY_END();
}
