/**
 * @file fsm.c
 * @brief The garage door state machine.
 */
#include <assert.h>
#include <stddef.h>

#include "fsm.h"

typedef struct {
    state_t next;
    action_t action;
} transition_t;

static const transition_t table[N_STATES][N_EVENTS] = {
    [CLOSED] = {
        [E_REMOTE_PRESSED]          = { OPENING, A_NONE },
        [E_REMOTE_PRESSED_FREEZING] = { CLOSED, A_LOG_REFUSAL },
        [E_BARRIER_BLOCKED]         = { CLOSED, A_NONE },
        [E_TIMER_EXPIRED]           = { CLOSED, A_NONE }
    },
    [OPENING] = {
        [E_REMOTE_PRESSED]          = { CLOSING, A_NONE },
        [E_REMOTE_PRESSED_FREEZING] = { CLOSING, A_NONE },
        [E_BARRIER_BLOCKED]         = { OPENING, A_NONE },
        [E_TIMER_EXPIRED]           = { OPEN, A_NONE }
    },
    [OPEN] = {
        [E_REMOTE_PRESSED]          = { CLOSING, A_NONE },
        [E_REMOTE_PRESSED_FREEZING] = { CLOSING, A_NONE },
        [E_BARRIER_BLOCKED]         = { OPEN, A_NONE },
        [E_TIMER_EXPIRED]           = { OPEN, A_NONE }
    },
    [CLOSING] = {
        [E_REMOTE_PRESSED]          = { OPENING, A_NONE },
        [E_REMOTE_PRESSED_FREEZING] = { OPENING, A_NONE },
        [E_BARRIER_BLOCKED]         = { OBSTRUCTED, A_LOG_OBSTRUCTION },
        [E_TIMER_EXPIRED]           = { CLOSED, A_NONE }
    },
    [OBSTRUCTED] = {
        [E_REMOTE_PRESSED]          = { OBSTRUCTED, A_NONE },
        [E_REMOTE_PRESSED_FREEZING] = { OBSTRUCTED, A_NONE },
        [E_BARRIER_BLOCKED]         = { OBSTRUCTED, A_NONE },
        [E_TIMER_EXPIRED]           = { OPENING, A_NONE }
    }
};

static const char *const state_names[] = {
    [CLOSED]     = "CLOSED",
    [OPENING]    = "OPENING",
    [OPEN]       = "OPEN",
    [CLOSING]    = "CLOSING",
    [OBSTRUCTED] = "OBSTRUCTED",
};

_Static_assert(
    sizeof state_names / sizeof state_names[0] == N_STATES,
    "state_names is out of step with state_t"
);

const char *fsm_state_name(state_t state) {
    assert(state >= 0 && state < N_STATES);

    return state_names[state];
}

state_t fsm_step(state_t state, event_t event, action_t *action) {
    assert(state >= 0 && state < N_STATES);
    assert(event >= 0 && event < N_EVENTS);
    assert(action != NULL);

    const transition_t t = table[state][event];

    *action = t.action;
    return t.next;
}
