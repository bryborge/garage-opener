# Garage Door Opener Specification

This document records the design decisions for the garage door opener.

## Decisions

The assignment's three operating rules leave several cases open. Each decision below is encoded in the transition table in `src/fsm.c`.

### 1. Remote pressed while door is in motion

Pressing the remote while the garage door is moving in either direction reverse the door's movement. If it's `OPENING`, the remote press sets it to `CLOSING`, and vis versa.

### 2. Freeze Guard

The freeze guard exists to prevent the garage door sticking to a frozen floor. This means that it only applies when the door is `CLOSED` and it is at or below 1.0 °C. In `OPENING`, `OPEN`, or `CLOSING`, it behaves exactly like a press above 1.0 °C.

### 3. Light Barrier while the door is not closing

The light barrier exists to prevent someone or something getting crushed under the weight of the garage door while it's `CLOSING`. Its ignored in `CLOSED`, `OPENING`, `OPEN`, and `OBSTRUCTED`.

### 4. Remote pressed while obstructed

A press is ignored for the 2 ticks the garage door spends in `OBSTRUCTED`. And its not logged as a refusal, because the press is ignored on account of the obstruction, which is logged.
