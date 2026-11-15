/**
 * @file sensor.h
 * @brief The temperature and humidity sensor.
 */
#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>
#include <time.h>

typedef struct {
    time_t timestamp;
    float temp_c;
    float rh_pct;
} reading_t;

/**
 * Take a sensor reading. Temperature and humidity are read from the registers
 * defined in iom361, simulating a memory-mapped AHT20 device.
 *
 * @param base The address to the full 32-bit register.
 * @param now  Time of this reading, UTC seconds since the Unix epoch.
 * @param out  Where the record is stored.
 * @return 0 on success, or else the iom361 return code of the register read
 *         that fails.
 */
int sensor_read(uint32_t *base, time_t now, reading_t *out);

#endif
