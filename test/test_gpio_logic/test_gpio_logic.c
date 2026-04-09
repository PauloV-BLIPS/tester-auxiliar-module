#include <unity.h>
#include "gpio_logic.h"
#include <string.h>

// ── Pin validation ─────────────────────────────────────────────────

void test_pin_valid_mem2(void) {
    TEST_ASSERT_TRUE(pin_is_valid(PIN_MEM2));
}

void test_pin_valid_block(void) {
    TEST_ASSERT_TRUE(pin_is_valid(PIN_BLOCK));
}

void test_pin_invalid_zero(void) {
    TEST_ASSERT_FALSE(pin_is_valid(0));
}

void test_pin_invalid_negative(void) {
    TEST_ASSERT_FALSE(pin_is_valid(-1));
}

void test_pin_invalid_high(void) {
    TEST_ASSERT_FALSE(pin_is_valid(99));
}

// ── Pin writability ────────────────────────────────────────────────

void test_pin_writable_mem2(void) {
    TEST_ASSERT_TRUE(pin_is_writable(PIN_MEM2));
}

void test_pin_not_writable_block(void) {
    TEST_ASSERT_FALSE(pin_is_writable(PIN_BLOCK));
}

void test_pin_not_writable_invalid(void) {
    TEST_ASSERT_FALSE(pin_is_writable(99));
}

// ── Pin state ──────────────────────────────────────────────────────

void test_state_init_zeroed(void) {
    PinStateStore s;
    pin_state_init(&s);
    for (int i = 0; i < NUM_PINS; i++) {
        TEST_ASSERT_EQUAL_INT(0, s.value[i]);
        TEST_ASSERT_EQUAL_UINT32(0, s.timestamp_ms[i]);
    }
}

void test_state_set_stores_value(void) {
    PinStateStore s;
    pin_state_init(&s);
    pin_state_set(&s, PIN_MEM2, 1, 1000);
    TEST_ASSERT_EQUAL_INT(1, s.value[PIN_MEM2]);
    TEST_ASSERT_EQUAL_UINT32(1000, s.timestamp_ms[PIN_MEM2]);
}

void test_state_update_changed(void) {
    PinStateStore s;
    pin_state_init(&s);
    bool changed = pin_state_update(&s, PIN_BLOCK, 1, 2000);
    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_INT(1, s.value[PIN_BLOCK]);
    TEST_ASSERT_EQUAL_UINT32(2000, s.timestamp_ms[PIN_BLOCK]);
}

void test_state_update_unchanged(void) {
    PinStateStore s;
    pin_state_init(&s);
    pin_state_update(&s, PIN_BLOCK, 1, 1000);
    bool changed = pin_state_update(&s, PIN_BLOCK, 1, 2000);
    TEST_ASSERT_FALSE(changed);
    TEST_ASSERT_EQUAL_UINT32(1000, s.timestamp_ms[PIN_BLOCK]);
}

void test_state_update_out_of_bounds(void) {
    PinStateStore s;
    pin_state_init(&s);
    TEST_ASSERT_FALSE(pin_state_update(&s, 99, 1, 1000));
}

void test_state_set_out_of_bounds(void) {
    PinStateStore s;
    pin_state_init(&s);
    pin_state_set(&s, -1, 1, 1000);
    for (int i = 0; i < NUM_PINS; i++) {
        TEST_ASSERT_EQUAL_INT(0, s.value[i]);
    }
}

// ── JSON formatters ────────────────────────────────────────────────

void test_fmt_json_read(void) {
    char buf[96];
    fmt_json_read(buf, sizeof(buf), 1, 1, 5000);
    TEST_ASSERT_EQUAL_STRING("{\"pin\":1,\"value\":1,\"timestamp_ms\":5000}", buf);
}

void test_fmt_json_read_zero(void) {
    char buf[96];
    fmt_json_read(buf, sizeof(buf), 4, 0, 0);
    TEST_ASSERT_EQUAL_STRING("{\"pin\":4,\"value\":0,\"timestamp_ms\":0}", buf);
}

void test_fmt_json_write(void) {
    char buf[48];
    fmt_json_write(buf, sizeof(buf), 1, 0);
    TEST_ASSERT_EQUAL_STRING("{\"pin\":1,\"value\":0}", buf);
}

void test_fmt_json_error(void) {
    char buf[128];
    fmt_json_error(buf, sizeof(buf), "test msg");
    TEST_ASSERT_EQUAL_STRING("{\"error\":\"test msg\"}", buf);
}

void test_fmt_json_status(void) {
    char buf[64];
    fmt_json_status(buf, sizeof(buf), 12345, 9999);
    TEST_ASSERT_EQUAL_STRING("{\"heap\":12345,\"uptime_ms\":9999}", buf);
}

void test_fmt_json_error_truncation(void) {
    char buf[10];
    int needed = fmt_json_error(buf, sizeof(buf), "a long message");
    TEST_ASSERT_GREATER_THAN(9, needed);
    TEST_ASSERT_EQUAL_UINT8('\0', buf[9]);
}

// ── Write validation ───────────────────────────────────────────────

void test_write_valid(void) {
    TEST_ASSERT_EQUAL(WRITE_OK, validate_write_params(true, true, PIN_MEM2));
}

void test_write_missing_pin(void) {
    TEST_ASSERT_EQUAL(WRITE_ERR_PIN_MISSING, validate_write_params(false, true, 0));
}

void test_write_missing_value(void) {
    TEST_ASSERT_EQUAL(WRITE_ERR_VALUE_MISSING, validate_write_params(true, false, PIN_MEM2));
}

void test_write_pin_not_writable(void) {
    TEST_ASSERT_EQUAL(WRITE_ERR_PIN_NOT_WRITABLE, validate_write_params(true, true, PIN_BLOCK));
}

// ── Runner ─────────────────────────────────────────────────────────

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_pin_valid_mem2);
    RUN_TEST(test_pin_valid_block);
    RUN_TEST(test_pin_invalid_zero);
    RUN_TEST(test_pin_invalid_negative);
    RUN_TEST(test_pin_invalid_high);

    RUN_TEST(test_pin_writable_mem2);
    RUN_TEST(test_pin_not_writable_block);
    RUN_TEST(test_pin_not_writable_invalid);

    RUN_TEST(test_state_init_zeroed);
    RUN_TEST(test_state_set_stores_value);
    RUN_TEST(test_state_update_changed);
    RUN_TEST(test_state_update_unchanged);
    RUN_TEST(test_state_update_out_of_bounds);
    RUN_TEST(test_state_set_out_of_bounds);

    RUN_TEST(test_fmt_json_read);
    RUN_TEST(test_fmt_json_read_zero);
    RUN_TEST(test_fmt_json_write);
    RUN_TEST(test_fmt_json_error);
    RUN_TEST(test_fmt_json_status);
    RUN_TEST(test_fmt_json_error_truncation);

    RUN_TEST(test_write_valid);
    RUN_TEST(test_write_missing_pin);
    RUN_TEST(test_write_missing_value);
    RUN_TEST(test_write_pin_not_writable);

    return UNITY_END();
}
