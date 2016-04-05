#include "thread.h"

static int last_pid;

/* exit() analog */
void thread_exit(void) {
    local_irq_disable();
    list_del(&current_thread->threads_list);

    /**
     * I can free memory and after that call `schedule()`
     * because slab allocator does not spoil this memory,
     * I don't do allocations here and I disabled interrupts.
     * Nothing can go wrong if in schedule I use current_fread
     * which was freed
     */
    kmem_free((char *)current_thread->rbp - STACK_SIZE + 1);
    kmem_free(current_thread);

    schedule();
}

static void main_function_wrapper(void) {
    current_thread->fptr(current_thread->arg);
    thread_exit();
}

/**
 * I use current rflags except for
 * interrupt flag and status flags.
 * I set interrupt flag and clear
 * status flags. See wiki for details.
 */
static uint64_t make_rflags(void) {
    uint64_t current_rflags = 0;

    __asm__ volatile ("pushf");
    __asm__ volatile ("popq %rax");
    __asm__ volatile ("movq %%rax, %0" : "=r"(current_rflags));

    uint64_t to_clear = UINTMAX_MAX - BIT_CONST(0) - BIT_CONST(2) -
        BIT_CONST(4) - BIT_CONST(6) - BIT_CONST(7) - BIT_CONST(10) -
        BIT_CONST(11);

    current_rflags = current_rflags & to_clear;

    return current_rflags | INTERRUPT_FLAG;
}

static void setup_stack(thread_desc_t *thread_desc) {
    struct thread_regs *regs = (struct thread_regs *)thread_desc->rsp;
    /**
     * thread_regs also has intno, error, etc.
     * we don't use them at all so they wont
     * affect us. 
     *
     * Just ignore them as I don't want to create
     * new struct which would be almost the same
     * as `struct thread_regs` except this 
     * additional fields.
     *
     * There is no need to initialize r12-r15 and rbx.
     * I do it just to be more pedantic.
     */
    regs->r15 = 0;
    regs->r14 = 0;
    regs->r13 = 0;
    regs->r12 = 0;

    regs->rbx = 0;
    regs->rbp = (uint64_t)thread_desc->rbp;

    regs->rflags = make_rflags();

    uint64_t *return_address = (uint64_t *)((char *)thread_desc->rsp + 160);
    *(return_address) = (uint64_t)&main_function_wrapper;
}

pid_t create_thread(void (*fptr)(void *), void *arg) {
    void *new_stack = kmem_alloc(STACK_SIZE);
    thread_desc_t *thread_desc = (thread_desc_t *)kmem_alloc(sizeof(thread_desc_t));

    thread_desc->pid  = ++last_pid;
    thread_desc->rbp  = (char *)new_stack + STACK_SIZE - 1; //stack beginning
    thread_desc->rsp  = (char *)thread_desc->rbp + 1; // + 1 because of push firstly decrement and after that mov
    thread_desc->fptr = fptr;
    thread_desc->arg  = arg;

    /**
     * we make -168 because we need to store registers on the stack
     * (160 mostly because of %rflags)
     * and we also need to store return address (as switch_threads
     * makes `retq` instruction). This address should be the address
     * of our main_function_wrapper.
     */
    thread_desc->rsp = (char *)thread_desc->rsp - 168;

    setup_stack(thread_desc); 

    list_add_tail(&thread_desc->threads_list, &init.threads_list);

    return thread_desc->pid;
}

void setup_threads(void) {
    init.pid = ++last_pid;
    list_init(&init.threads_list);

    current_thread = &init;
}

/**
 * In this realization you can not join
 * init process. I know about this and
 * do this for purpose.
 */
int pid_in_list(pid_t pid) {
    thread_desc_t *desc =
        LIST_ENTRY(init.threads_list.next, thread_desc_t, threads_list);

    while (desc->pid != 1) {
        if (desc->pid == pid)
            return 1;
        else
            desc = LIST_ENTRY(desc->threads_list.next, thread_desc_t, threads_list);
    }

    return 0;
}
