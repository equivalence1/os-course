#ifndef __THREAD_H__
#define __THREAD_H__

#include "list.h"
#include "memory.h"
#include "interrupt.h"
#include "scheduler.h"
#include "kmem_cache.h"
#include "thread_regs.h"

#include <stdint.h>

/** 
 * 256K for each thread.
 * if it's not enough we should use
 * buddy allocator or thread can 
 * allocate more space by itself
 */
#define STACK_SIZE (1024) //TODO need large

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
pid_t create_thread(void (*fptr)(void *), void *arg);

#endif // __THREAD_H__
