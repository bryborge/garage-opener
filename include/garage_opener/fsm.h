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

/* Transitions */
typedef struct {
    state_t next;
    action_t action;
} transition_t;

#endif
