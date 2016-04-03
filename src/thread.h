#ifndef __THREAD_H__
#define __THREAD_H__

#include "list.h"
#include "memory.h"
#include "kernel.h"
#include "interrupt.h"
#include "scheduler.h"
#include "kmem_cache.h"
#include "thread_regs.h"

#include <stdint.h>

#define STACK_SIZE (4 * 1024) //4K for each thread

typedef int pid_t;

struct thread_desc {
    pid_t pid;
    void *rbp;
    void *rsp;
    void (*fptr)(void *);
    void *arg;

    struct list_head threads_list;
};
typedef struct thread_desc thread_desc_t;

/* global cyclic queue of all currently alive threads */
thread_desc_t init;
thread_desc_t *current_thread;

void setup_threads(void);
int pid_in_list(pid_t);
pid_t create_thread(void (*fptr)(void *), void *arg);

#endif // __THREAD_H__
