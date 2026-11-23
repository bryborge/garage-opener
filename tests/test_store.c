/**
 * @file test_store.c
 * @brief Unit tests for the record store module.
 */
#include "unity.h"

#include "store.h"

/*
 * Each test gets a new, empty store. tearDown destroys it, and Unity runs
 * tearDown even after a failed assertion, so a failing test does not leak.
 */
static store_t *store;

void setUp(void) {
    store = store_create();
    TEST_ASSERT_NOT_NULL(store);
}

void tearDown(void) {
    store_destroy(store);
    store = NULL;
}

/**
 * Two temperatures in the same 0.5 °C step have the same key.
 */
static void test_record_compare_same_step_is_equal(void) {
    const record_t a = { .reading = { .temp_c = 21.2f } };
    const record_t b = { .reading = { .temp_c = 21.4f } };

    TEST_ASSERT_EQUAL_INT(0, record_compare(&a, &b));
    TEST_ASSERT_EQUAL_INT(0, record_compare(&b, &a));
}

/**
 * Two temperatures in neighboring steps are ordered by step, even when they
 * are only 0.1 °C apart.
 */
static void test_record_compare_neighboring_steps_are_ordered(void) {
    const record_t lower = { .reading = { .temp_c = 21.4f } };
    const record_t upper = { .reading = { .temp_c = 21.5f } };

    TEST_ASSERT_LESS_THAN_INT(0, record_compare(&lower, &upper));
    TEST_ASSERT_GREATER_THAN_INT(0, record_compare(&upper, &lower));
}

/**
 * A temperature exactly on a step boundary starts the step above it. The
 * boundary at 0.0 °C also checks that negative temperatures round down:
 * -0.1 °C is in the step from -0.5 °C, not the step from 0.0 °C.
 */
static void test_record_compare_boundary_starts_the_step_above(void) {
    const record_t below = { .reading = { .temp_c = -0.1f } };
    const record_t boundary = { .reading = { .temp_c = 0.0f } };
    const record_t inside = { .reading = { .temp_c = 0.4f } };

    TEST_ASSERT_LESS_THAN_INT(0, record_compare(&below, &boundary));
    TEST_ASSERT_EQUAL_INT(0, record_compare(&boundary, &inside));
}

/**
 * A new store holds no records and has a height of 0.
 */
static void test_store_create_starts_empty(void) {
    TEST_ASSERT_EQUAL_size_t(0, store_count(store));
    TEST_ASSERT_EQUAL_size_t(0, store_height(store));
}

/**
 * Inserting into an empty store adds one record.
 */
static void test_store_insert_into_empty_store(void) {
    const record_t record = { .reading = { .temp_c = 21.2f } };

    TEST_ASSERT_EQUAL_INT(0, store_insert(store, &record));
    TEST_ASSERT_EQUAL_size_t(1, store_count(store));
}

/**
 * The count includes every record, whether its key is new or shared, while
 * the height counts only nodes, one per key. The 100 records span 10 keys, 10
 * records each, so every node's array grows past its starting room. The first
 * 10 arrive in increasing order, so the 10 nodes form a chain: height 10.
 */
static void test_store_count_and_height_after_1_2_and_100_inserts(void) {
    for (int i = 0; i < 100; i++) {
        /* 15.0, 15.5, ..., 19.5 °C, then around again. */
        const record_t record = {
            .reading = { .temp_c = 15.0f + (float)(i % 10) * 0.5f },
        };
        TEST_ASSERT_EQUAL_INT(0, store_insert(store, &record));

        if (i == 0) {
            TEST_ASSERT_EQUAL_size_t(1, store_count(store));
            TEST_ASSERT_EQUAL_size_t(1, store_height(store));
        }
        if (i == 1) {
            TEST_ASSERT_EQUAL_size_t(2, store_count(store));
            TEST_ASSERT_EQUAL_size_t(2, store_height(store));
        }
    }

    TEST_ASSERT_EQUAL_size_t(100, store_count(store));
    TEST_ASSERT_EQUAL_size_t(10, store_height(store));
}

/**
 * A long run at one temperature, the usual case in a replay, fills one node
 * and leaves the tree's height at 1.
 */
static void test_store_one_temperature_stays_one_node_tall(void) {
    const record_t record = { .reading = { .temp_c = 15.0f } };

    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_INT(0, store_insert(store, &record));
    }

    TEST_ASSERT_EQUAL_size_t(100, store_count(store));
    TEST_ASSERT_EQUAL_size_t(1, store_height(store));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_record_compare_same_step_is_equal);
    RUN_TEST(test_record_compare_neighboring_steps_are_ordered);
    RUN_TEST(test_record_compare_boundary_starts_the_step_above);
    RUN_TEST(test_store_create_starts_empty);
    RUN_TEST(test_store_insert_into_empty_store);
    RUN_TEST(test_store_count_and_height_after_1_2_and_100_inserts);
    RUN_TEST(test_store_one_temperature_stays_one_node_tall);

    return UNITY_END();
}
