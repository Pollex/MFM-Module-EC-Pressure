//
// Created by Eric van Rijswick on 29/01/2024.
//

#include "perif/huba713.h"
#include "drivers/zacwire.h"
#include <avr/interrupt.h>
#include <avr/io.h>

// struct huba713_t huba713_config;

void huba713_init(void) { zacwire_init(); }

/// @brief Read a pressure and temperature (3-bytes total) from the bus
/// @details Read a pressure and temperature (3-bytes total) from the (32kHz)
/// bus
/// @param pressure pointer to the pressure data
/// @param temperature pointer to the temperature data
/// @return validity of the data: 0x00 if valid, 0xFF if invalid
uint8_t huba713_read(uint16_t *pressure, float *temperature) {
    // Note, 1-wire for this Huba pressure sensor is 32kHz, so 31.3us periode
    // Wait for the bus to become idle

    uint8_t buf[3] = {0};
    // The bus is idle, wait for start

    int err = zacwire_read(buf, 3);

    *pressure = (buf[0] << 8) | buf[1];
    *temperature = (0.784 * (float)buf[2]) - 50;

    return err;
}
