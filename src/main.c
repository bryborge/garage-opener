/**
 * @file main.c
 * @brief The garage door opener entrypoint.
 */
#include <stdio.h>
#include <inttypes.h>

#include "iom361_r4.h"
#include "sensor.h"

#define START_TIME ((time_t) 1791273600)  // Placeholder clock

int main(void) {
    /* Initialize iom361 */
    int rc;
    uint32_t *base = iom361_initialize(2, 2, &rc);

    if (base == NULL) {
        fprintf(stderr, "iom361_initialize failed: %d\n", rc);
        return 1;
    }

    /* World Setup */
    _iom361_set_sensor1(15.0f, 60.0f);
    _iom361_set_switches(0x1);

    uint32_t sw = iom361_readReg(base, SWITCHES_REG, &rc);

    reading_t reading;
    rc = sensor_read(base, START_TIME, &reading);

    if (rc != 0) {
        fprintf(stderr, "sensor_read failed: %d\n", rc);
        return 1;
    }

    if (sw & 0x1u) {  // REMOTE is switch 0
        iom361_writeReg(base, LEDS_REG, 0x1u, &rc);
    }

    iom361_writeReg(base, RGB_LED_REG, build_rgb_reg(1, 255, 0, 0), &rc);

    printf("temp %.1f C, rh %.1f %%RH, switches 0x%" PRIX32 "\n",
        reading.temp_c, reading.rh_pct, sw);

    return 0;
}
