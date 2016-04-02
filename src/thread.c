#include "thread.h"

static int last_pid;

/* exit() analog*/
static void thread_exit(void) {
    /**
     * we need to disable interrupts here
     * as we are changing current_thread
     * and thread_queue. Interrupt after
     * `list_del` and before `schedule`
     * will destroy everything
     */
    local_irq_disable();

    list_del(&current_thread->threads_list);

    kmem_free((char *)current_thread->rbp - STACK_SIZE + 1);
    kmem_free(current_thread);

    schedule();

    local_irq_enable();
}

#include "stdio.h"

static void main_function_wrapper(void) {
    current_thread->fptr(current_thread->arg);
    while (1) {
        printf("123\n");
        schedule();
    }
    thread_exit();
}

static uint64_t get_current_rflag(void) {
    return 1L << 9; // TODO DON'T KNOW
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
     */
/* TODO delete it
    regs->r15 = 0;
    regs->r14 = 0;
    regs->r13 = 0;
    regs->r12 = 0;
    regs->r11 = 0;
    regs->r10 = 0;
    regs->r9  = 0;
    regs->r8  = 0;

    regs->rax = 0;
    regs->rbx = 0;
    regs->rcx = 0;
    regs->rdx = 0;

    regs->rdi = 0;
    regs->rsi = 0;
*/
    regs->r15 = 0;
    regs->r14 = 0;
    regs->r13 = 0;
    regs->r12 = 0;

    regs->rbp = (uint64_t)thread_desc->rbp;
    regs->rbx = 0;

    regs->rflags = get_current_rflag();

    uint64_t *return_address = (uint64_t *)((char *)thread_desc->rsp + 160);
    *(return_address) = (uint64_t)&main_function_wrapper;
}

pid_t create_thread(void (*fptr)(void *), void *arg) {
    printf("creating 1\n");
    void *new_stack = kmem_alloc(STACK_SIZE);
    thread_desc_t *thread_desc = (thread_desc_t *)kmem_alloc(sizeof(thread_desc)); //TODO don't forget to free

    printf("creating 2\n");
    thread_desc->pid  = ++last_pid;
    thread_desc->rbp  = (char *)new_stack + STACK_SIZE - 1; //stack beginning
    thread_desc->rsp  = (char *)thread_desc->rbp + 1; // + 1 because of push firstly decrement and after that mov
    thread_desc->fptr = fptr;
    thread_desc->arg  = arg;

    printf("creating 3\n");
    /**
     * we make -168 because we need to store registers on the stack
     * (160 mostly because of %rflags)
     * and we also need to store return address (as switch_threads
     * makes `retq` instruction). This address should be the address
     * of our main_function_wrapper.
     */
    thread_desc->rsp = (char *)thread_desc->rsp - 168;
    printf("pid %d sp is %#llx\n", thread_desc->pid, thread_desc->rsp);

    printf("creating 4\n");
    setup_stack(thread_desc); 

    printf("creating 5\n");
    list_add_tail(&thread_desc->threads_list, &init.threads_list);

    printf("creating 6\n");
    return thread_desc->pid;
}

void setup_threads(void) {
    init.pid = ++last_pid;
    list_init(&init.threads_list);

    current_thread = &init;
}
