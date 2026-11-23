# Garage Door Opener Specification

This document records the design decisions for the garage door opener.

## Decisions

The assignment's three operating rules leave several cases open. The decisions about the remote, the freeze guard, the light barrier, and timing are implemented in `src/fsm.c`. The decisions about the event history are implemented in `src/main.c`.

### Remote

#### Remote pressed while door is in motion

Pressing the remote while the garage door is moving in either direction reverses the door's movement. If it's `OPENING`, the remote press sets it to `CLOSING`, and vice versa.

#### Remote pressed while obstructed

Pressing a button on the remote is ignored for the 2 ticks the garage door spends in `OBSTRUCTED`. And it's not logged as a refusal, because the press is ignored on account of the obstruction, which is logged. The press itself is still recorded in the event history.

### Freeze Guard

#### Purpose

The freeze guard exists to prevent the garage door sticking to a frozen floor. This means that it only applies when the door is `CLOSED` and it is at or below 1.0 °C. In `OPENING`, `OPEN`, or `CLOSING`, it behaves exactly like a press above 1.0 °C.

#### Freeze guard status

The status word has a bit that shows whether the freeze guard is active. The guard is active when the garage door is `CLOSED` and it is at or below 1.0 °C. This is checked at the end of each tick.

### Light Barrier

#### Light barrier while the door is not closing

The light barrier exists to prevent someone or something getting crushed under the weight of the garage door while it's `CLOSING`. It's ignored in `CLOSED`, `OPENING`, `OPEN`, and `OBSTRUCTED`.

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

### Event history

#### Refused press

A refused remote press is recorded as two entries: the press, and then the refusal. Both carry the same tick. The press is what the remote did, and the refusal is what the machine decided to do about it. If the refusal was recorded without a remote press recorded as well, someone reading the history would see refusals with no presses before them and reasonably ask, "is this a malfunction? Did it stop registering button presses?" Recording both means every press shows up in the history, whatever the machine did with it. The refusal goes on second, so it ends up on top. Cause, then effect.

#### Order of entries within a tick

When more than one thing happens on the same tick, the entries go on in the order the machine handled them: the obstruction, then the press, then the refusal. This is the same order as the machine itself (see [Order of events within a tick](#order-of-events-within-a-tick)). An obstruction and a refusal can never happen on the same tick, because a refusal needs the door `CLOSED` and an obstruction needs it `CLOSING`.

#### Printing the history on an obstruction

The last 3 entries are printed once, on the tick the garage door becomes `OBSTRUCTED`, right after that tick's trace line. They are not printed again on the ticks the door spends paused. The obstruction is the event worth explaining, and printing the same entries every tick would just be noise. The entries are printed most recent first, one per line:

```
2026-10-06 08:00:28 state=OBSTRUCTED temp=15.0 rh=60.0 leds=0x0 status=0x00000004
  history tick=28 event=OBSTRUCTION
  history tick=25 event=PRESS
  history tick=9 event=PRESS
```

The obstruction is recorded before the printout, so it is always the first of the 3. If the history holds fewer than 3 entries, all of them are printed.

#### Reading below the top

A stack only lets you see its top. To print 3 entries, `main` pops them off and then pushes them back in reverse order, which leaves the history exactly as it was. This keeps the history a stack. Adding a function to look at any position would have been simpler, but it would have turned the stack into an array with extra steps.

#### When the history can't grow

If a push fails because the history is full and more memory can't be allocated, `main` reports it and the replay stops. This is the same as every other failure during a tick. Carrying on would leave a history that no longer matches what happened, and anything printed from it afterward would be wrong without anyone knowing.

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

## Status word

The status word is one 32-bit value that summarizes the machine on a tick. It is printed in the trace and stored with each record.

| Bits | Field |
|---|---|
| 0 to 2 | The state |
| 3 | Unused |
| 4 | `MOTOR_UP` is on |
| 5 | `MOTOR_DOWN` is on |
| 6 | The freeze guard is active |
| 7 to 31 | Unused |

Unused bits are always 0.

The state is stored as its number:

| State | Value |
|---|---|
| `CLOSED` | 0 |
| `OPENING` | 1 |
| `OPEN` | 2 |
| `CLOSING` | 3 |
| `OBSTRUCTED` | 4 |

Bit 3 is left unused intentionally. It puts the state in the low hex digit and the three flags in the next one, so the word can be read without decoding it. For example:

| Status word | Meaning |
|---|---|
| `0x00000011` | `OPENING`, with `MOTOR_UP` on |
| `0x00000023` | `CLOSING`, with `MOTOR_DOWN` on |
| `0x00000040` | `CLOSED`, with the freeze guard active |

## Event history

The event history is a stack of entries, most recent on top. Every remote press, refusal, and obstruction is pushed with the tick it happened on.

### How the stack grows

The stack is a dynamic array. It starts with room for 8 entries. When it's full and another entry is pushed, the room doubles: 8, then 16, then 32, and so on. Popping never gives room back, so the room only ever grows to the most the history has held at once.

Doubling is what keeps a push cheap. Growing means copying every entry into a bigger block, which costs time in proportion to how many entries there are. If the room grew by one each time, every push would copy everything, and a push would cost time in proportion to the size of the history. Doubling means a copy happens only at 8, 16, 32, and so on. After n pushes, all the copies added together come to fewer than 2n entries, so on average each push costs a constant amount. Not giving room back on a pop keeps this simple. If a history shrank and grew again, a run of pushes and pops right at a boundary would copy the whole array over and over.

The array is a separate block from the `history_t` that holds it. When the array grows, `realloc` may move it to a new address. Because only the array moves, the `history_t *` that `main` holds stays valid for the life of the history.

### Compared with a linked list

The other common way to build a stack is a linked list, where each entry sits in its own node with a pointer to the one below it, and the top is the head of the list.

| Operation | Dynamic array | Linked list |
|---|---|---|
| Push | O(1) averaged; O(n) on the push that doubles the room | O(1), but one `malloc` every time |
| Pop | O(1) | O(1), but one `free` every time |
| Peek | O(1) | O(1) |
| Count | O(1), from the count field | O(1) if a count is kept, otherwise O(n) |
| Destroy | O(1): two `free` calls | O(n): one `free` per node |
| Memory per entry | 8 bytes, with up to half the room unused right after a doubling | 16 bytes per node (the entry and its pointer), which `malloc` turns into a 24-byte block on this computer, plus its own bookkeeping |

The linked list's one real advantage is that no push ever has to copy anything, so every push takes the same time. That would matter if the device had a hard deadline on every tick. This one doesn't. The array wins on everything else. It uses less memory. It allocates rarely instead of on every push, so there are only a few points where memory can run out, and the heap doesn't fill up with small blocks. The entries also sit next to each other in memory, which makes reading them faster. And the history mostly grows. Pops happen only while printing, and those entries are pushed straight back, so the list's cheap pop doesn't buy anything here.

## The year 2038

A signed 32-bit `time_t` can count up to 2,147,483,647 seconds after the Unix epoch, which is 2038-01-19 03:14:07 UTC. The next tick does not fit. Signed overflow is undefined in C, but in practice the value wraps around to the most negative number, which is 1901-12-13 20:45:52 UTC.

For this device, that means the timestamps would be wrong and everything else would carry on:

- The trace would jump from 2038-01-19 03:14:07 back to 1901-12-13 20:45:52 and keep counting from there.
- Every reading stored after that would carry a date in 1901, so the records would no longer be in time order.
- The garage door itself would behave normally. The state machine counts ticks (the door position and the pause) and never looks at the timestamp, so opening, closing, and the obstruction pause would take as long as they always do.

On the computer this was built on, `time_t` is 64 bits, so the problem does not occur.
