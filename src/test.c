#include "test.h"

static void print_thread_num(void *arg) {
    (void)sizeof(arg);
    printf("hello from pid %d\n", current_thread->pid);

    if (current_thread->pid % 2 == 0) {
        printf("pid %d exiting without yield() call\n", current_thread->pid);
    } else {
        printf("pid %d exiting with yield() call\n", current_thread->pid);
        for (int i = 0; i < 5; i++) // shoud make a little delay on screen
            yield();
    }
    printf("goodbuy from pid %d\n", current_thread->pid);
}

static spinlock_t *lock1;
static spinlock_t *lock2;

static void locking_print(void *arg) {
    int num = (uint64_t)arg;

    printf("started locking print by %d with arg %d\n", current_thread->pid, num);

    if (num == 1)
        lock(lock1);
    else
        lock(lock2);

    printf("locking print with arg %d done\n", num);

    if (num == 1)
        unlock(lock1);
    else
        unlock(lock2);
}

static void infinite_circle(void *arg) {
    (void)sizeof(arg);

    printf("starting \"infinite\" cirle\n");
    int x = 0;
    while (1) {
        x++;
        if (x % 10000000 == 0)
            thread_exit();
    }
}

static void infinite_circle1(void *arg) {
    (void)sizeof(arg);

    printf("starting \"infinite\" cirle\n");
    int x = 0;
    while (1) {
        x++;
        if (x % 100000000 == 0)
            thread_exit();
    }
}

void test_threads() {
    lock1 = (spinlock_t *)kmem_alloc(sizeof(spinlock_t));
    lock1->users = 0;
    lock1->ticket = 0;
    lock2 = (spinlock_t *)kmem_alloc(sizeof(spinlock_t));
    lock2->users = 0;
    lock2->ticket = 0;

#define N_THREADS 5
    printf("test begin, threads in queue %d\n", list_size(&current_thread->threads_list));

    for (int i = 0; i < N_THREADS; i++) {
        create_thread(&print_thread_num, NULL);
    }

    for (int i = 0; i < N_THREADS; i++) {
        join(i + 2);
    }

    lock(lock1);
    lock(lock2);

    int l1pid = create_thread(&locking_print, (void *)1);

    unlock(lock2);

    int l2pid = create_thread(&locking_print, (void *)2);
    join(l2pid);

    unlock(lock1);
    join(l1pid);

    // after that we should see second print before first
    // even though first is before second in threads queue

    int inf_pid = create_thread(&infinite_circle, NULL);
    create_thread(&infinite_circle1, NULL);
    join(inf_pid);

    printf("test end, threads in queue %d\n", list_size(&current_thread->threads_list));
#undef N_THREADS

    kmem_free(lock1);
    kmem_free(lock2);
}
