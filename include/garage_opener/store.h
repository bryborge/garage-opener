/**
 * @file store.h
 * @brief The record store.
 */
#ifndef STORE_H
#define STORE_H

#include <stdint.h>

#include "fsm.h"
#include "sensor.h"

typedef struct {
    reading_t reading;
    state_t state;
    uint32_t status;
} record_t;

#endif
