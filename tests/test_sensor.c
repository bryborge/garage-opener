/**
 * @file test_sensor.c
 * @brief Unit tests for the sensor module.
 */
#include <stdint.h>
#include <time.h>

#include "iom361_r4.h"
#include "sensor.h"
#include "unity.h"

static uint32_t *base;

void setUp(void) {
    int rc;
    base = iom361_initialize(2, 2, &rc);

    TEST_ASSERT_NOT_NULL(base);
}

void tearDown(void) {}

/**
 * sensor_read() fills every field of type reading.
 */
static void test_sensor_read_fills_reading_at_0_c(void) {
    const time_t now = 1791273600;  // 2026-10-06 08:00:00 UTC

    _iom361_set_sensor1(0.0f, 50.0f);

    reading_t reading;
    int rc = sensor_read(base, now, &reading);

    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_INT64(now, reading.timestamp);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, reading.temp_c);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, reading.rh_pct);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sensor_read_fills_reading_at_0_c);
    return UNITY_END();
}
