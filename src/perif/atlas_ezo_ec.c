//
// Created by Eric van Rijswick on 31/01/2024.
//

#include "perif/atlas_ezo_ec.h"
#include "mcu/uart.h"
#include <string.h>
#include <avr/io.h>
#include "board/mfm_sensor_module.h"
#include <util/delay.h>
#include <mcu/util.h>
#include <util/atomic.h>

/// @brief Initialize the Atlas Scientific EZO EC
void atlas_ezo_ec_init(void)
{
  uart_init();
}

/// @brief Disable the Atlas Scientific EZO EC
/// @details This function will disable the Atlas Scientific EZO EC by setting the enable pin as input
/// The external pull-down will turn the isolator board off
void atlas_ezo_ec_disable() {
  // Set enable pin as input; pull-down will turn the isolator board off
  ENABLE_CONDUCTIVITY_PORT.DIR &= ~ENABLE_CONDUCTIVITY_PIN;
  uart_disable();
}

/// @brief Enable the Atlas Scientific EZO EC
/// @details This function will enable the Atlas Scientific EZO EC by setting the enable pin high
void atlas_ezo_ec_enable() {
  // Set enable pin as input; pull-up will turn the isolator board off
  ENABLE_CONDUCTIVITY_PORT.DIR |= ENABLE_CONDUCTIVITY_PIN;
  ENABLE_CONDUCTIVITY_PORT.OUT |= ENABLE_CONDUCTIVITY_PIN;
  uart_init();
  atlas_ezo_ec_waitForBoot();
}

/// @brief Send command to the Atlas Scientific EZO EC
/// @param cmd Pointer to a string that holds the command
static void atlas_ezo_ec_sendCommandAndWaitForResponse(char *cmd)
{
  uart_sendString(cmd);
  uart_readline();
}

#define TIMEOUT 100 // Timeout in milliseconds

/// @brief Request value from the Atlas Scientific EZO EC
/// @param value Pointer to a string that will hold the value
/// @return 0 if successful, -1 if not
int atlas_ezo_ec_requestValue(char *value) {
  // Read while there is still data in the buffer to flush the buffer
  while (USART0.STATUS & USART_RXCIF_bm) {
    uint8_t h = USART0.RXDATAH;
    uint8_t l = USART0.RXDATAL;
  }

  // Send command to request value
  uart_sendString("R\r");

  // The expected response is x.xx\r*OK\r
  uint8_t response[12];
  uint8_t c;
  uint8_t ii;
  uint8_t done = 0;

  // Get the start time
  uint32_t start_time = millis();

  // Read characters until the end of the response is reached
  while (done == 0) {
    // Check if the timeout has been reached
    if (millis() - start_time > TIMEOUT) {
      return -1; // Return an error code
    }

    // start with the value, which is 4 characters and a carriage return
    for (ii = 0; ii < 4; ii++) {
      c = uart_read();

      // If the carriage return is reached something went wrong, so start reading the status
      if (c == 0x0D) {
        done = 0x01;
        break;
      }
      value[ii] = c;
    }

    // Read while there is still data in the buffer to flush the buffer
    while (USART0.STATUS & USART_RXCIF_bm) {
      uint8_t h = USART0.RXDATAH;
      uint8_t l = USART0.RXDATAL;
    }
  }
  return 0;
}

/// @brief Disable continuous reading from the Atlas Scientific EZO EC
/// @return 0 if successful, -1 if not
int atlas_ezo_ec_disableContinuousReading(void)
{
  uart_sendString("C,0\r");
  char *response = uart_readline();
  if (strcmp(response, "*OK") == 0) {
    return 0;
  }
  return -1;
}

/// @brief Wait for the Atlas Scientific EZO EC to boot
/// @return 0 if successful, -1 if not
int atlas_ezo_ec_waitForBoot(void) {
  // Wait on ready
  uint8_t ready = 0;
  while (ready == 0) {
    char *response = uart_readline();
    if (strcmp(response, "*RE") == 0) {
      ready = 0x01;
    }
  }

  if (ready == 0x01) {
    return 0;
  }
}
