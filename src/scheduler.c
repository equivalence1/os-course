#include "scheduler.h"

extern void switch_threads(void **old_sp, void *new_sp);

#include "stdio.h"

void schedule(void) {
    void **old_sp = &current_thread->rsp;

    /**
     * We need to disable interrupts here because otherwise
     * it is possible that we changed global `current_thread`
     * value and before calling `switch_threads()` interrupt
     * was generated by PIT. In this way `current_thread`
     * would become irrelevant. We should prevent such 
     * situations. Easy way to do this -- just disable 
     * interrups and enable after everything was done.
     */


    printf("current pid %d\n", current_thread->pid);
    current_thread = 
        LIST_ENTRY(current_thread->threads_list.next, thread_desc_t, threads_list);
    printf("new pid %d\n", current_thread->pid);

    //TODO
    //if int here we have a huge fail

    if (*old_sp != current_thread->rsp) {
        printf("here\n");
        switch_threads(old_sp, current_thread->rsp);
    }
}
