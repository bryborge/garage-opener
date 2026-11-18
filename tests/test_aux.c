/**
 * @file test_aux.c
 * @brief Unit tests for the event history module.
 */
#include <stddef.h>

#include "unity.h"

#include "aux.h"

/*
 * The test is linked with -Wl,--wrap=realloc, so every call to realloc in
 * aux.c comes here instead. The wrapper records the call and passes it on to
 * the real realloc, which the linker names __real_realloc. This makes the
 * growth of the hidden array visible.
 */
static size_t realloc_calls;
static size_t realloc_last_size;

void *__real_realloc(void *ptr, size_t size);

void *__wrap_realloc(void *ptr, size_t size) {
    realloc_calls++;
    realloc_last_size = size;

    return __real_realloc(ptr, size);
}

/*
 * Each test gets a new, empty history. tearDown destroys it, and Unity runs
 * tearDown even after a failed assertion, so a failing test does not leak.
 */
static history_t *history;

void setUp(void) {
    realloc_calls = 0;
    realloc_last_size = 0;

    history = history_create();
    TEST_ASSERT_NOT_NULL(history);
}

void tearDown(void) {
    history_destroy(history);
    history = NULL;
}

/**
 * A new history has no entries.
 */
static void test_history_create_starts_empty(void) {
    TEST_ASSERT_EQUAL_size_t(0, history_count(history));
}

/**
 * A push adds one entry.
 */
static void test_history_push_adds_an_entry(void) {
    const history_entry_t entry = { .kind = H_PRESS, .tick = 5 };
    TEST_ASSERT_EQUAL_INT(0, history_push(history, entry));
    TEST_ASSERT_EQUAL_size_t(1, history_count(history));
}

/**
 * A new history has room for 8 entries, and the room doubles each time a push
 * finds it full: to 16 on the 9th push and to 32 on the 17th.
 */
static void test_history_push_doubles_room_when_full(void) {
    int tick = 0;

    /* Pushes 1 to 8 fit in the starting room. */
    for (; tick < 8; tick++) {
        const history_entry_t entry = { .kind = H_PRESS, .tick = tick };
        TEST_ASSERT_EQUAL_INT(0, history_push(history, entry));
    }
    TEST_ASSERT_EQUAL_size_t(0, realloc_calls);

    /* Push 9 doubles the room to 16. */
    const history_entry_t ninth = { .kind = H_PRESS, .tick = tick++ };
    TEST_ASSERT_EQUAL_INT(0, history_push(history, ninth));
    TEST_ASSERT_EQUAL_size_t(1, realloc_calls);
    TEST_ASSERT_EQUAL_size_t(16 * sizeof(history_entry_t), realloc_last_size);

    /* Pushes 10 to 16 fit in the new room. */
    for (; tick < 16; tick++) {
        const history_entry_t entry = { .kind = H_PRESS, .tick = tick };
        TEST_ASSERT_EQUAL_INT(0, history_push(history, entry));
    }
    TEST_ASSERT_EQUAL_size_t(1, realloc_calls);

    /* Push 17 doubles the room to 32. */
    const history_entry_t seventeenth = { .kind = H_PRESS, .tick = tick++ };
    TEST_ASSERT_EQUAL_INT(0, history_push(history, seventeenth));
    TEST_ASSERT_EQUAL_size_t(2, realloc_calls);
    TEST_ASSERT_EQUAL_size_t(32 * sizeof(history_entry_t), realloc_last_size);

    TEST_ASSERT_EQUAL_size_t(17, history_count(history));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_history_create_starts_empty);
    RUN_TEST(test_history_push_adds_an_entry);
    RUN_TEST(test_history_push_doubles_room_when_full);

    return UNITY_END();
}
