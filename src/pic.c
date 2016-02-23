#include "pic.h"
#include "uart.h"

void set_up_pic(void) {
    out8(MASTER_PIC_COMMAND, FIRST_WORD);
    out8(MASTER_PIC_DATA, ICW2_MASTER);
    out8(MASTER_PIC_DATA, CONFIG_WORD_MASTER);
    out8(MASTER_PIC_DATA, MODE_WORD);

    out8(SLAVE_PIC_COMMAND, FIRST_WORD);
    out8(SLAVE_PIC_DATA, ICW2_SLAVE);
    out8(SLAVE_PIC_DATA, CONFIG_WORD_SLAVE);
    out8(SLAVE_PIC_DATA, MODE_WORD);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        out8(SLAVE_PIC_COMMAND, PIC_EOI);
    }
    out8(MASTER_PIC_COMMAND, PIC_EOI);
}

void IRQ0_handler_c() {
    uart_puts("irq0_handler_c\n");
    pic_send_eoi(0);
}
