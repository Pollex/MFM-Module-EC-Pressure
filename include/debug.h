#if !defined(_DEBUG_H_)
#define _DEBUG_H_

#include <stdio.h>
#include <stdint.h>
#include "mcu/uart.h"

void my_printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

#ifdef __cplusplus
extern "C"
{
#endif

  char buf[32] = {0};
  void uart_printf(uart_t *uart, const char *fmt, ...)
  {
    va_list args;
    va_start(args, fmt);
    sprintf(buf, fmt, args);
    va_end(args);
    uart_print(uart, buf);
  }

#ifdef __cplusplus
}
#endif

#endif // _DEBUG_H_