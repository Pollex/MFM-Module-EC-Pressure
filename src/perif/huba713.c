//
// Created by Eric van Rijswick on 29/01/2024.
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include "perif/huba713.h"
#include "drivers/zacwire.h"

//struct huba713_t huba713_config;

void huba713_init(void) {
   zacwire_init();
}


/// @brief Read a pressure and temperature (3-bytes total) from the bus
/// @details Read a pressure and temperature (3-bytes total) from the (32kHz) bus
/// @param pressure pointer to the pressure data
/// @param temperature pointer to the temperature data
/// @return validity of the data: 0x00 if valid, 0xFF if invalid
uint8_t huba713_read(uint16_t *pressure, float *temperature) {
  cli();
  // Note, 1-wire for this Huba pressure sensor is 32kHz, so 31.3us periode
  // Wait for the bus to become idle

  zacwire_waitForIdle();

  uint8_t byte1 = 0;
  uint8_t byte2 = 0;
  uint8_t byte3 = 0;
  // The bus is idle, wait for start

  uint8_t parity1 = zacwire_readByte(&byte1);
  uint8_t parity2 = zacwire_readByte(&byte2);
  uint8_t parity3 = zacwire_readByte(&byte3);

  *pressure = (byte1 << 8) | byte2;
  *temperature = (0.784 * (float)byte3) -50;

  if (parity1 == 0x00 && parity2 == 0x00 && parity3 == 0x00) {
    sei();
    return 0x00;
  }
  else {
    sei();
    return 0xFF;
  }
}