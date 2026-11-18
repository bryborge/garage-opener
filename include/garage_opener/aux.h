/**
 * @file aux.h
 * @brief The event history.
 */
#ifndef AUX_H
#define AUX_H

#include <stddef.h>

typedef enum {
    H_PRESS,                    // The remote was pressed.
    H_REFUSAL,                  // A remote press was refused.
    H_OBSTRUCTION,              // The door was obstructed.
} history_kind_t;

typedef struct {
    history_kind_t kind;
    int tick;                   // The tick it happened on.
} history_entry_t;

/**
 * The history: a stack of entries.
 */
typedef struct history history_t;

/**
 * Create an empty history with room for 8 entries. The room doubles each time
 * the history is full and another entry is pushed.
 *
 * @return The history, or NULL if memory could not be allocated.
 */
history_t *history_create(void);

/**
 * Destroy a history and every entry still in it. Does nothing if history is
 * NULL.
 *
 * @param history The history to destroy.
 */
void history_destroy(history_t *history);

/**
 * Count the entries in a history. Asserts that history is not NULL.
 *
 * @param history The history to count.
 * @return The number of entries.
 */
size_t history_count(const history_t *history);

/**
 * Push an entry onto the top of a history. If the history is full, its room
 * doubles first. Asserts that history is not NULL.
 *
 * @param history The history to push onto.
 * @param entry   The entry to push.
 * @return 0 on success, or 1 if the history was full and more memory could
 *         not be allocated. The history is unchanged on failure.
 */
int history_push(history_t *history, history_entry_t entry);

#endif
