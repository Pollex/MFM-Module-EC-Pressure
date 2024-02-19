#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "drivers/zacwire.h"
#include "board/mfm_sensor_module.h"

static uint8_t calculate_parity(uint8_t data);

/// @brief Initialize the zacwire bus
void zacwire_init() {
  // Set the pin as input
  ZACWIRE_PORT.DIR &= ~ZACWIRE_PIN;
}

/// @brief Wait for a start bit
/// @details The start bit is a falling edge followed by a rising edge with a 50% duty cycle;
/// the duty cycle is ignored in this implementation
/// @condition The bus is idle
void zacwire_waitForStart(void) {

  // All bits start low, so wait for first low
  while (ZACWIRE_PORT.IN & ZACWIRE_PIN);

  // wait for the pin to go high to signal start
  while (!(ZACWIRE_PORT.IN & ZACWIRE_PIN));

  // The next bit start low, so wait for first low
  while (ZACWIRE_PORT.IN & ZACWIRE_PIN);
}



uint8_t calculate_parity(uint8_t data) {
  uint8_t parity = 0;
  uint8_t byte = data;
  while (byte) {
    parity ^= byte & 1;
    byte >>= 1;
  }
  return parity;
}

#define DELAY_US 2

/// @brief Read a byte from the bus
/// @details Read a byte from the (32kHz) bus
/// @param data pointer to the data byte
/// @return validity of the data: 0x00 if valid, 0xFF if invalid
/// @condition The bus is idle
uint8_t zacwire_readByte(uint8_t *data) {

  zacwire_waitForStart();

//  TEST_PORT.OUT &= ~TEST_PIN;

    // sample every 16us (half the bit time or at least between 25-50% of the bit time)
    *data = 0;
    for (int ii = 0; ii < 8; ii++) {
        *data <<= 1; // Shift the data left
        if (ii == 0) {
          _delay_us(9 + DELAY_US); // Wait for half the bit time
        }
        else {
          _delay_us(11 + DELAY_US); // Wait for half the bit time
        }

//      TEST_PORT.OUT &= ~TEST_PIN;

      // Store value
        if (ZACWIRE_PORT.IN & ZACWIRE_PIN) {
          *data |= 0x01; // Set the most significant bit
        } else {
          *data &= ~0x01; // Set the most significant bit
        }
        // Wait half the bit time
        _delay_us(11 + DELAY_US); // Wait for half the bit time
//        TEST_PORT.OUT |= TEST_PIN;

    }
    // data contains 8 bits data

    // get the parity
    _delay_us(12 + DELAY_US); // Wait for half the bit time
    uint8_t parity = 0x00;
//    TEST_PORT.OUT &= ~TEST_PIN;

    if (ZACWIRE_PORT.IN & ZACWIRE_PIN) {// If the pin is high
      parity = 0x01; // Positive parity
    }
    _delay_us(9 + DELAY_US); // Wait for half the bit time

//    TEST_PORT.OUT &= ~TEST_PIN;

    // wait for the stop bit, bus goes high
    while (!(ZACWIRE_PORT.IN & ZACWIRE_PIN));
//    TEST_PORT.OUT |= TEST_PIN; // Toggle the test pin

    return (parity == calculate_parity(*data)) ? 0x00 : 0xFF;
}


/// @brief Wait for the bus to be idle
void zacwire_waitForIdle() {
  // wait for idle time (spec is 1ms, so lets wait for > 500us; stop bit is >> 32us)
  int count = 0;

  while (count < 100) {
    count++;

    if (ZACWIRE_PORT.IN & ZACWIRE_PIN) {
      // bus is high (state needed for idle)
      count++;

    }
    else {
      // bus is low, not idle
      count = 0;
    }
    _delay_us(5);
  }
}