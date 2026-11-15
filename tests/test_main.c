/**
 * @file test_main.c
 * @brief Test suite for main.
 */
#include <stdint.h>

#include "iom361_r4.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

/*
 * Smoke test: after iom361_initialize() the sensor registers hold the
 * documented defaults, 23.5 C and 75 %RH (docs/SPEC.md section 2). The
 * registers are 20-bit, so a value comes back within 0.0001 of what was set.
 */
static void test_initialize_sets_default_sensor_reading(void) {
    int rc;
    uint32_t *base = iom361_initialize(2, 2, &rc);

    TEST_ASSERT_NOT_NULL(base);

    uint32_t raw_temp = iom361_readReg(base, TEMP_REG, &rc);
    TEST_ASSERT_EQUAL_INT(0, rc);
    double temp_c = raw_temp / 1048576.0 * 200.0 - 50.0;
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 23.5, temp_c);

    uint32_t raw_rh = iom361_readReg(base, HUMID_REG, &rc);
    TEST_ASSERT_EQUAL_INT(0, rc);
    double rh_pct = raw_rh / 1048576.0 * 100.0;
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 75.0, rh_pct);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_initialize_sets_default_sensor_reading);
    return UNITY_END();
}
