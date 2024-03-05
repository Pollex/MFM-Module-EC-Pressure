//
// Created by Eric van Rijswick on 31/01/2024.
//

#include "perif/atlas_ezo_ec.h"
#include "board/mfm_sensor_module.h"
#include "mcu/uart.h"
#include <avr/io.h>
#include <mcu/util.h>
#include <stdio.h>
#include <string.h>
#include <util/atomic.h>
#include <util/delay.h>

/// @brief Initialize the Atlas Scientific EZO EC
void atlas_ezo_ec_init(void) { uart_init(); }

/// @brief Disable the Atlas Scientific EZO EC
/// @details This function will disable the Atlas Scientific EZO EC by setting
/// the enable pin as input The external pull-down will turn the isolator board
/// off
void atlas_ezo_ec_disable() {
    // Set enable pin as input; pull-down will turn the isolator board off
    ENABLE_CONDUCTIVITY_PORT.DIR &= ~ENABLE_CONDUCTIVITY_PIN;
    uart_disable();
}

/// @brief Enable the Atlas Scientific EZO EC
/// @details This function will enable the Atlas Scientific EZO EC by setting
/// the enable pin high
void atlas_ezo_ec_enable() {
    // Set enable pin as input; pull-up will turn the isolator board off
    ENABLE_CONDUCTIVITY_PORT.DIR |= ENABLE_CONDUCTIVITY_PIN;
    ENABLE_CONDUCTIVITY_PORT.OUT |= ENABLE_CONDUCTIVITY_PIN;
    uart_init();
    atlas_ezo_ec_waitForBoot();
}

/// @brief Send command to the Atlas Scientific EZO EC
/// @param cmd Pointer to a string that holds the command
static void atlas_ezo_ec_sendCommandAndWaitForResponse(char *cmd) {
    uart_sendString(cmd);
    uart_readline();
}

#define TIMEOUT 100 // Timeout in milliseconds

/// @brief Request value from the Atlas Scientific EZO EC
/// @param value Pointer to a string that will hold the value, size should be
/// fixed to 8 characters
/// @return 0 if successful, -1 if not
int atlas_ezo_ec_requestValue(uint8_t *value) {
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
    uint8_t done = 0;

    // Read characters from ec sensor until a carriage return is received or the
    // buffer is full We don't know how many characters we will receive, but we
    // have a fixed buffer size of 8, so move the characters to the end of the
    // buffer and pad with zeros
    uint8_t nofChars = 0;
    char tmp[8];
    do {
        c = uart_read();

        // Store every character except a carriage return
        if (c != 0x0D) {
            tmp[nofChars] = c;
            nofChars++;
        }
    } while ((c != 0x0D) && (nofChars < 8));

    // Fill value with zeros for padding
    for (int ii = 0; ii < 8; ii++) {
        value[ii] = 0;
    }

    // Now lets move the received characters to the end of the buffer and pad
    // with zeros
    for (int ii = 0; ii < nofChars; ii++) {
        value[8 - nofChars + ii] = tmp[ii];
    }

    // Read while there is still data in the buffer to flush the buffer
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t h = USART0.RXDATAH;
        uint8_t l = USART0.RXDATAL;
    }

    return 0;
}

/// @brief Disable continuous reading from the Atlas Scientific EZO EC
/// @return 0 if successful, -1 if not
int atlas_ezo_ec_disableContinuousReading(void) {
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
    return -1;
}

int atlas_ezo_ec_calibrate(void) {
    // Send command to calibrate
    uart_sendString("Cal,dry\r");
    char *response = uart_readline();
    if (strcmp(response, "*OK") == 0) {
        return 0;
    }
    return -1;
}

int atlas_ezo_ec_setTemperature(uint8_t t) {
    // T,255\r\0   = 7 characters
    static char buf[10];
    sprintf(buf, "T,%d\r", t);
    uart_sendString(buf);
    char *response = uart_readline();
    if (strcmp(response, "*OK") == 0) {
        return 0;
    }
    return -1;
}
