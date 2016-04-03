#include "scheduler.h"
#include "stdio.h" //TODO only for debug

extern void switch_threads(void **old_sp, void *new_sp);

void schedule(void) {
    void **old_sp = &current_thread->rsp;
    pid_t old_pid = current_thread->pid;

    current_thread = 
        LIST_ENTRY(current_thread->threads_list.next, thread_desc_t, threads_list);

    /**
     * if new PIT interrupt occured right here and next thread
     * exited, we can fail here. If threads call `yield` instead
     * of `schedule` this situation can not happen.
     */

    /* we can't schedule 1 thread (it will always be init) */
    if (current_thread->pid != old_pid) {
        // TODO this is only for debug.
        printf("changing <%d> on <%d>\n", old_pid, current_thread->pid);
        switch_threads(old_sp, current_thread->rsp);
    }
}

/**
 * threads should use this function instead of schedule
 * if interrupts enabled
 */
void yield(void) {
    local_irq_disable();
    schedule();
    local_irq_enable();
}

void join(pid_t pid) {
    if (current_thread->pid == pid)
        return; // thread can not join itself
    while (pid_in_list(pid));
}
