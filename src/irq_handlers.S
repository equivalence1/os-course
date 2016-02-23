    .code64
    .globl IRQ0_handler
    .extern IRQ0_handler_c

# gcc preserves rbp, rbx and r12-r15
.macro push_registers
    push %rax
    push %rcx
    push %rdx
    push %rdi
    push %rsi
    push %r8
    push %r9
    push %r10
    push %r11
.endm

.macro pop_registers
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rsi
    pop %rdi
    pop %rdx
    pop %rcx
    pop %rax
.endm

IRQ0_handler:
    push_registers

    cld #gcc requires this
    call IRQ0_handler_c

    pop_registers
    iretq
