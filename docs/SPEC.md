# Garage Door Opener Specification

This document records the design decisions for the garage door opener.

## Decisions

The assignment's three operating rules leave several cases open. Each decision below is implemented in `src/fsm.c`.

### Remote

#### Remote pressed while door is in motion

Pressing the remote while the garage door is moving in either direction reverse the door's movement. If it's `OPENING`, the remote press sets it to `CLOSING`, and vis versa.

#### Remote pressed while obstructed

A press is ignored for the 2 ticks the garage door spends in `OBSTRUCTED`. And its not logged as a refusal, because the press is ignored on account of the obstruction, which is logged.

### Freeze Guard

#### Purpose

The freeze guard exists to prevent the garage door sticking to a frozen floor. This means that it only applies when the door is `CLOSED` and it is at or below 1.0 °C. In `OPENING`, `OPEN`, or `CLOSING`, it behaves exactly like a press above 1.0 °C.

#### Freeze guard status

The status word has a bit that shows whether the freeze guard is active. The guard is active when the garage door is `CLOSED` and it is at or below 1.0 °C. This is checked at the end of each tick.

### Light Barrier

#### Light barrier while the door is not closing

The light barrier exists to prevent someone or something getting crushed under the weight of the garage door while it's `CLOSING`. Its ignored in `CLOSED`, `OPENING`, `OPEN`, and `OBSTRUCTED`.

### Timing

#### Door position

The machine tracks the garage door's position, from 0 (closed) to 15 (open). While the door opens, it gains one step per tick and while it closes it loses one step per tick. So a full open (or close) takes a full 15 ticks. When the door reverses travel, it goes back only the distance it has already traveled. For example, if the door has opened 5 steps, and a remote button press occurs, it closes again in 5 ticks. An obstructed door holds its position during the pause and reopens from where it stopped.

#### Order of events within a tick

Several things can happen on the same tick. The machine handles them in a fixed order:

1. The light barrier.
2. Completion: the garage door takes a step, and an obstructed door counts down its pause. Reaching the end of travel (or of the pause) completes the state.
3. The remote press.

The barrier comes first for safety. If the door is blocked on the tick it would have finished closing, it will stop instead. The press comes last so that the machine reacts to what is physically happening before it interprets a command. One consequence is that a door still takes its step on the tick of a press, and then reverses.

#### First tick in a new state

When the garage door enters a new state, nothing more happens to it on that tick. It starts moving (or counting down its pause) on the next tick. For example, if the remote is pressed on tick 10, the door is `OPENING` on tick 10, takes its first step on tick 11, and is `OPEN` on tick 25. The same goes for an obstruction. The door stops where it is, and its 2-tick pause starts counting on the next tick.

## Events

The state machine reacts to events, not to raw inputs. Each tick, the inputs are turned into events as follows.

| Event | Produced when |
|---|---|
| `E_REMOTE_PRESSED` | The remote switch went from off to on, and it is above 1.0 °C |
| `E_REMOTE_PRESSED_FREEZING` | The remote switch went from off to on, and it is at or below 1.0 °C |
| `E_BARRIER_BLOCKED` | The light barrier switch is on. It is a level, so the event is produced on every tick the barrier stays blocked |
| `E_TIMER_EXPIRED` | A moving door reached the end of its travel, or an obstructed door reached the end of its pause |

A remote that is held on produces one press. It must be released and turned on again to produce a second.

## Transition table

| State | Event | Next state | Action |
|---|---|---|---|
| `CLOSED` | `E_REMOTE_PRESSED` | `OPENING` | `A_NONE` |
| `CLOSED` | `E_REMOTE_PRESSED_FREEZING` | `CLOSED` | `A_LOG_REFUSAL` |
| `CLOSED` | `E_BARRIER_BLOCKED` | `CLOSED` | `A_NONE` |
| `CLOSED` | `E_TIMER_EXPIRED` | `CLOSED` | `A_NONE` |
| `OPENING` | `E_REMOTE_PRESSED` | `CLOSING` | `A_NONE` |
| `OPENING` | `E_REMOTE_PRESSED_FREEZING` | `CLOSING` | `A_NONE` |
| `OPENING` | `E_BARRIER_BLOCKED` | `OPENING` | `A_NONE` |
| `OPENING` | `E_TIMER_EXPIRED` | `OPEN` | `A_NONE` |
| `OPEN` | `E_REMOTE_PRESSED` | `CLOSING` | `A_NONE` |
| `OPEN` | `E_REMOTE_PRESSED_FREEZING` | `CLOSING` | `A_NONE` |
| `OPEN` | `E_BARRIER_BLOCKED` | `OPEN` | `A_NONE` |
| `OPEN` | `E_TIMER_EXPIRED` | `OPEN` | `A_NONE` |
| `CLOSING` | `E_REMOTE_PRESSED` | `OPENING` | `A_NONE` |
| `CLOSING` | `E_REMOTE_PRESSED_FREEZING` | `OPENING` | `A_NONE` |
| `CLOSING` | `E_BARRIER_BLOCKED` | `OBSTRUCTED` | `A_LOG_OBSTRUCTION` |
| `CLOSING` | `E_TIMER_EXPIRED` | `CLOSED` | `A_NONE` |
| `OBSTRUCTED` | `E_REMOTE_PRESSED` | `OBSTRUCTED` | `A_NONE` |
| `OBSTRUCTED` | `E_REMOTE_PRESSED_FREEZING` | `OBSTRUCTED` | `A_NONE` |
| `OBSTRUCTED` | `E_BARRIER_BLOCKED` | `OBSTRUCTED` | `A_NONE` |
| `OBSTRUCTED` | `E_TIMER_EXPIRED` | `OPENING` | `A_NONE` |

The actions are:

| Action | Meaning |
|---|---|
| `A_NONE` | Nothing is recorded |
| `A_LOG_REFUSAL` | A refused remote press is recorded |
| `A_LOG_OBSTRUCTION` | An obstruction is recorded |

## Outputs

The motor LEDs and the RGB LED depend only on the state.

| State | `MOTOR_UP` | `MOTOR_DOWN` | Color | Red, green, blue |
|---|---|---|---|---|
| `CLOSED` | off | off | Green | 0, 255, 0 |
| `OPENING` | on | off | Yellow | 255, 255, 0 |
| `OPEN` | off | off | Blue | 0, 0, 255 |
| `CLOSING` | off | on | Magenta | 255, 0, 255 |
| `OBSTRUCTED` | off | off | Red | 255, 0, 0 |
