#include "ioport.h"
#include "pic.h"
#include "pit.h"
#include "interrupt.h"
#include "uart.h"

extern void IRQ0_handler(void);

void main(void) { 
    set_up_pic();
    set_up_idt();
    set_up_pit();
    set_up_uart();

    __asm__ volatile ("sti");

    while (1) {
        __asm__ volatile ("hlt");
    }
}
