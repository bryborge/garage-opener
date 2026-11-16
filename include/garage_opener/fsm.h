/**
 * @file fsm.h
 * @brief The garage door state machine.
 */
#ifndef FSM_H
#define FSM_H

/* States */
typedef enum {
    CLOSED,                     // The door is closed (Initial state).
    OPENING,                    // The door is opening.
    OPEN,                       // The door is open.
    CLOSING,                    // The door is closing.
    OBSTRUCTED,                 // The door is obstructed by something.
    N_STATES
} state_t;

/* Events */
typedef enum {
    E_REMOTE_PRESSED,           // The remote was pressed, temp > 1.0 C.
    E_REMOTE_PRESSED_FREEZING,  // The remote was pressed, temp <= 1.0 C.
    E_BARRIER_BLOCKED,          // The light barrier is triggered.
    E_TIMER_EXPIRED,            // The current state's time is up.
    N_EVENTS
} event_t;

/* Actions */
typedef enum {
    A_UNDEFINED,                // Empty? (used only in tests)
    A_NONE,                     // Do nothing.
    A_LOG_REFUSAL,              // Record a refused remote press.
    A_LOG_OBSTRUCTION,          // Record an obstruction.
    N_ACTIONS
} action_t;

/**
 * Get the name of a state. Asserts that state is in range.
 *
 * @param state The state to name.
 * @return A string that is valid for the life of the program.
 */
const char *fsm_state_name(state_t state);

/**
 * Look up the transition for an event occurring in a state. The machine keeps
 * no state of its own: the caller holds the current state and stores the one
 * returned. Asserts that state and event are in range and action is not NULL.
 *
 * @param state  The current state of the machine.
 * @param event  The event that occurred.
 * @param action Where to store the action to take.
 * @return The next state.
 */
state_t fsm_step(state_t state, event_t event, action_t *action);

#endif
