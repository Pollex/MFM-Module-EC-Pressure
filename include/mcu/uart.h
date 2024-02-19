//
// Created by Eric van Rijswick on 31/01/2024.
//

#ifndef USART_H
#define USART_H
#include <avr/io.h>

#define MAX_RX_LINE_LENGTH 10

void uart_init(void);
void uart_sendChar(char c);
void uart_sendString(char *str);
char* uart_readline(void);
void uart_disable(void);
uint8_t uart_read(void);

#endif // USART_H
