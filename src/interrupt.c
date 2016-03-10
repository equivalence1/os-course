#include "interrupt.h"

#define INT_AND_PRESENT_FLAG 0x8E // 0b10001110

idt_desc_t idt[256];
idt_ptr_t  idt_ptr;

// extern is useless here. Just for clarity
extern void IRQ0_handler(void);

void set_idt_gate(uint8_t num, uint64_t base, uint16_t selector, uint8_t flags) {
    idt[num].offset_low  = get_bits(base, 0, 16);
    idt[num].offset_mid  = get_bits(base, 16, 16);
    idt[num].offset_high = get_bits(base, 32, 32);
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

idt_desc_t* set_up_idt(void) {
    idt_ptr.size = sizeof(idt_desc_t) * 256 - 1;
    idt_ptr.base = (uint64_t) idt;
    
    set_idt(&idt_ptr);

    set_idt_gate(32, (uint64_t) IRQ0_handler, KERNEL_CODE, INT_AND_PRESENT_FLAG);

    return idt;
}
