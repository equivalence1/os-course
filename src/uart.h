#ifndef __UART_H__
#define __UART_H__

#include "ioport.h"

#define UART_FIRST_PORT      0x3F8
#define UART_LCR_DIV_MODE    0x83 // 0b10000011
#define UART_LCR_INT_MODE    0x03
#define UART_DIVISOR         1

void set_up_uart(void);
void uart_puts(const char* str);

#endif // __UART_H__
