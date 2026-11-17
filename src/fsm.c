/**
 * @file fsm.c
 * @brief The garage door state machine.
 */
#include <assert.h>
#include <stddef.h>

#include "fsm.h"

static const int travel_ticks         = 15;
static const int pause_ticks          = 2;
static const float freeze_threshold_c = 1.0f;

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

state_t fsm_step(state_t state, event_t event, action_t *action) {
    assert(state >= 0 && state < N_STATES);
    assert(event >= 0 && event < N_EVENTS);
    assert(action != NULL);

    const transition_t t = table[state][event];

    *action = t.action;
    return t.next;
}

fsm_t fsm_init(void) {
    return (fsm_t){
        .state            = CLOSED,
        .remote_was_on    = false,
        .position         = 0,
        .pause_ticks_left = 0,
    };
}

/**
 * Feed one event to the transition table, then update the machine and the
 * report from the result.
 */
static void apply(fsm_t *fsm, event_t event, fsm_report_t *report) {
    action_t action;
    const state_t next = fsm_step(fsm->state, event, &action);

    switch (action) {
        case A_NONE:
            break;
        case A_LOG_REFUSAL:
            report->press_refused = true;
            break;
        case A_LOG_OBSTRUCTION:
            report->obstruction_detected = true;
            break;
        default:
            assert(false && "transition has no action");
            break;
    }

    /* The pause starts when the door becomes obstructed. */
    if (next == OBSTRUCTED && fsm->state != OBSTRUCTED) {
        fsm->pause_ticks_left = pause_ticks;
    }

    fsm->state = next;
}

void fsm_tick(fsm_t *fsm, const fsm_inputs_t *inputs, fsm_report_t *report) {
    assert(fsm != NULL);
    assert(inputs != NULL);
    assert(report != NULL);

    *report = (fsm_report_t){0};

    const bool freezing = inputs->temp_c <= freeze_threshold_c;
    const state_t starting_state = fsm->state;

    /* 1. Barrier check. */
    if (inputs->barrier_blocked) {
        apply(fsm, E_BARRIER_BLOCKED, report);
    }

    /* 2. Completion check. */
    if (fsm->state == starting_state) {
        bool complete = false;

        switch (fsm->state) {
            case OPENING:
                if (fsm->position < travel_ticks) fsm->position++;
                complete = fsm->position == travel_ticks;
                break;
            case CLOSING:
                if (fsm->position > 0) fsm->position--;
                complete = fsm->position == 0;
                break;
            case OBSTRUCTED:
                if (fsm->pause_ticks_left > 0) fsm->pause_ticks_left--;
                complete = fsm->pause_ticks_left == 0;
                break;
            default:
                break;
        }

        if (complete) apply(fsm, E_TIMER_EXPIRED, report);
    }

    /* 3. Remote check. on now, off before. */
    const bool pressed = inputs->remote_on && !fsm->remote_was_on;
    fsm->remote_was_on = inputs->remote_on;

    if (pressed) {
        report->remote_pressed = true;
        apply(fsm,
            freezing ? E_REMOTE_PRESSED_FREEZING : E_REMOTE_PRESSED,
            report);
    }

    report->freeze_guard_active = freezing && fsm->state == CLOSED;
}

static const fsm_outputs_t outputs[] = {
    /*               up     down   red  green  blue */
    [CLOSED]     = { false, false,   0,  255,    0 },  // Green
    [OPENING]    = { true,  false, 255,  255,    0 },  // Yellow
    [OPEN]       = { false, false,   0,    0,  255 },  // Blue
    [CLOSING]    = { false, true,  255,    0,  255 },  // Magenta
    [OBSTRUCTED] = { false, false, 255,    0,    0 },  // Red
};

_Static_assert(
    sizeof outputs / sizeof outputs[0] == N_STATES,
    "outputs are out of step with state_t"
);

fsm_outputs_t fsm_outputs(state_t state) {
    assert(state >= 0 && state < N_STATES);

    return outputs[state];
}

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
