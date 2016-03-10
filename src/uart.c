#include "uart.h"

void set_up_uart() {
    out8(UART_FIRST_PORT + 3, UART_LCR_DIV_MODE);
    out8(UART_FIRST_PORT + 0, get_bits(UART_DIVISOR, 0, 8));
    out8(UART_FIRST_PORT + 1, get_bits(UART_DIVISOR, 8, 8));

    out8(UART_FIRST_PORT + 3, UART_LCR_INT_MODE);
    out8(UART_FIRST_PORT + 1, 0);
}

void uart_puts(const char* str) {
    while (*(str) != 0) {
        while (!(in8(UART_FIRST_PORT + 5) & (1 << 5)));
        out8(UART_FIRST_PORT, *str);
        str++;
    }
}
