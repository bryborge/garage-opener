/**
 * @file main.c
 * @brief The garage door opener entrypoint.
 */
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

#include "iom361_r4.h"

#include "fsm.h"
#include "sensor.h"

#define START_TIME ((time_t) 1791273600)  // 2026-10-06 08:00:00 UTC
#define N_TICKS    30

/* Switch register bits. */
#define SW_REMOTE        (1u << 0)
#define SW_LIGHT_BARRIER (1u << 1)

/* LED register bits. */
#define LED_MOTOR_UP     (1u << 0)
#define LED_MOTOR_DOWN   (1u << 1)

/* A change to the world. From its tick on, the values hold until the next. */
typedef struct {
    int tick;
    float temp_c;
    float rh_pct;
    uint32_t switches;
} world_t;

/* Placeholder for a scenario file: open, start closing, block the barrier. */
static const world_t script[] = {
    {  0, 15.0f, 60.0f, 0x0 },
    {  2, 15.0f, 60.0f, 0x1 },  // REMOTE pressed
    {  3, 15.0f, 60.0f, 0x0 },  // and released
    { 19, 15.0f, 60.0f, 0x1 },  // REMOTE pressed
    { 20, 15.0f, 60.0f, 0x0 },  // and released
    { 23, 15.0f, 60.0f, 0x2 },  // LIGHT_BARRIER blocked
    { 24, 15.0f, 60.0f, 0x0 },  // and cleared
};

int main(void) {
    int rc;
    uint32_t *base = iom361_initialize(2, 2, &rc);

    if (base == NULL) {
        fprintf(stderr, "iom361_initialize failed: %d\n", rc);
        return 1;
    }

    /* Keep the model's LED display out of the trace. */
    iom361_set_display(false);

    const size_t n_changes = sizeof script / sizeof script[0];
    size_t next_change = 0;
    fsm_t fsm = fsm_init();

    for (int tick = 0; tick < N_TICKS; tick++) {
        /* Play the world. */
        if (next_change < n_changes && script[next_change].tick == tick) {
            const world_t *world = &script[next_change++];

            _iom361_set_sensor1(world->temp_c, world->rh_pct);
            _iom361_set_switches(world->switches);
        }

        /* Read the inputs. */
        reading_t reading;
        rc = sensor_read(base, START_TIME + tick, &reading);
        if (rc != 0) {
            fprintf(stderr, "sensor_read failed: %d\n", rc);
            return 1;
        }

        const uint32_t switches = iom361_readReg(base, SWITCHES_REG, &rc);
        if (rc != 0) {
            fprintf(stderr, "reading the switches failed: %d\n", rc);
            return 1;
        }

        /* Run the state machine. */
        const fsm_inputs_t inputs = {
            .remote_on       = (switches & SW_REMOTE) != 0,
            .barrier_blocked = (switches & SW_LIGHT_BARRIER) != 0,
            .temp_c          = reading.temp_c,
        };
        fsm_report_t report;  // Not used until the event history exists.
        fsm_tick(&fsm, &inputs, &report);

        /* Write the outputs. */
        const fsm_outputs_t outputs = fsm_outputs(fsm.state);
        const uint32_t leds = (outputs.motor_up ? LED_MOTOR_UP : 0u)
                            | (outputs.motor_down ? LED_MOTOR_DOWN : 0u);

        iom361_writeReg(base, LEDS_REG, leds, &rc);
        if (rc != 0) {
            fprintf(stderr, "writing the LEDs failed: %d\n", rc);
            return 1;
        }

        iom361_writeReg(base, RGB_LED_REG,
            build_rgb_reg(1, outputs.red, outputs.green, outputs.blue), &rc);
        if (rc != 0) {
            fprintf(stderr, "writing the RGB LED failed: %d\n", rc);
            return 1;
        }

        printf("tick=%-2d state=%-10s temp=%.1f rh=%.1f leds=0x%" PRIX32 "\n",
            tick, fsm_state_name(fsm.state), reading.temp_c, reading.rh_pct,
            leds);
    }

    return 0;
}
