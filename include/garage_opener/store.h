/**
 * @file store.h
 * @brief The record store.
 */
#ifndef STORE_H
#define STORE_H

#include <stddef.h>
#include <stdint.h>

#include "fsm.h"
#include "sensor.h"

typedef struct {
    reading_t reading;
    state_t state;
    uint32_t status;
} record_t;

/**
 * A binary search tree of records, ordered by key.
 */
typedef struct store store_t;

/**
 * Compare two records by key. The key is the temperature in 0.5 °C steps:
 * each step runs from a multiple of 0.5 °C up to, but not including, the next.
 * Asserts that no argument is NULL.
 *
 * @param a The first record.
 * @param b The second record.
 * @return A negative number, zero, or a positive number when a's key is less
 *         than, equal to, or greater than b's.
 */
int record_compare(const record_t *a, const record_t *b);

/**
 * Create an empty store.
 *
 * @return The store, or NULL if memory could not be allocated.
 */
store_t *store_create(void);

/**
 * Destroy a store and every record in it. Does nothing if store is NULL.
 *
 * @param store The store to destroy.
 */
void store_destroy(store_t *store);

/**
 * Count the records in a store, including every record that shares a key.
 * Asserts that store is not NULL.
 *
 * @param store The store to count.
 * @return The number of records.
 */
size_t store_count(const store_t *store);

/**
 * Insert a copy of a record. Records with the same key are kept in the order
 * they were inserted. Asserts that no argument is NULL.
 *
 * @param store  The store to insert into.
 * @param record The record to copy into the store.
 * @return 0 on success, or 1 if memory could not be allocated. The store is
 *         unchanged on failure.
 */
int store_insert(store_t *store, const record_t *record);

/**
 * Count the nodes along the longest path from the root down. Records that
 * share a key share a node, so they don't add to the height. An empty store
 * has a height of 0, and a store with one key has a height of 1. Asserts that
 * store is not NULL.
 *
 * @param store The store to measure.
 * @return The height of the tree.
 */
size_t store_height(const store_t *store);

#endif
