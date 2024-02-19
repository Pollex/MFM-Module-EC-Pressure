//
// Created by Eric van Rijswick on 31/01/2024.
//

#include <avr/io.h>

#include "board/mfm_sensor_module.h"
#include "mcu/uart.h"
#include <string.h>

/// @brief Calculate the baud rate register value
#define USART_BAUD_RATE(BAUD_RATE)     ((float)(64 * F_CPU / (16 * (float)BAUD_RATE)) + 0.5)

/// @brief Initialize the UART
/// @details This function will initialize the UART with the following settings:
/// - Baud rate: 9600
/// - Data bits: 8
/// - Parity: None
/// - Stop bits: 1
/// Default pins are used for the UART
void uart_init(void)
{
  USART0.BAUD = USART_BAUD_RATE(9600);

  USART_PORT.DIR &= ~USART_RX_PIN;
  USART_PORT.DIR |= USART_TX_PIN;

  USART0.CTRLB |= USART_TXEN_bm;
  USART0.CTRLB |= USART_RXEN_bm;

//  PORTMUX.CTRLB |= USART_DEFAULT_PINS; // select default pins for usart
//  PORTMUX.CTRLB = 0x01; // select alternate pins for usart
}

/// @brief Disable the UART by setting the RX and TX pins as input
void uart_disable() {
  USART0.CTRLB &= ~(USART_TXEN_bm | USART_RXEN_bm);
  USART_PORT.DIR &= ~USART_RX_PIN;
  USART_PORT.DIR &= ~USART_TX_PIN;
}

/// @brief Send a character over the UART
/// @param c The character to be sent
void uart_sendChar(char c)
{
  while(!(USART0.STATUS & USART_DREIF_bm))
  {
    ;
  }

  USART0.TXDATAL = c;
}

/// @brief Send a string over the UART
/// @param str Pointer to the string to be sent
void uart_sendString(char *str)
{
  for(size_t ii = 0; ii < strlen(str); ii++)
  {
    uart_sendChar(str[ii]);
  }
}

/// @brief Read a character from the UART
/// @return The received character
/// @Note This function will block until a character is received
uint8_t uart_read()
{
  while (!(USART0.STATUS & USART_RXCIF_bm))
  {
    ;
  }
  return USART0.RXDATAL;
}

/// @brief Read a line from the UART
/// @details This function will read a line from the UART and store it in a buffer. The last character will be '\0'
/// @return Pointer to the buffer
/// @Note This function will block until a carriage return is received or the maximum line length (10 chars) is reached
char* uart_readline() {
  static char line[MAX_RX_LINE_LENGTH];
  uint8_t i = 0;

  while (1) {
    while (!(USART0.STATUS & USART_RXCIF_bm)); // Wait for data to be received
    uint8_t data = USART0.RXDATAL; // Read the data

    if (data == 0x0D || i == MAX_RX_LINE_LENGTH - 1) { // If carriage return or end of buffer
      line[i] = '\0'; // Null-terminate the string
      break;
    } else {
      line[i++] = data; // Store the data in the buffer
    }
  }

  return line;
}
