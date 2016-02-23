#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "memory.h"

typedef struct idt_ptr {
	uint16_t size;
	uint64_t base;
} __attribute__((packed)) idt_ptr_t;

typedef struct idt_desc{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed)) idt_desc_t;

static inline void set_idt(const struct idt_ptr *ptr)
{ __asm__ volatile ("lidt (%0)" : : "a"(ptr)); }

idt_desc_t* set_up_idt(void);

#endif /*__INTERRUPT_H__*/
