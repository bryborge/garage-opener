/**
 * @file fsm.c
 * @brief The garage door state machine.
 */
#include "fsm.h"

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
