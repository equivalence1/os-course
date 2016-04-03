#include "locks.h"

void lock(struct spinlock *lock) {
    const uint16_t ticket = __sync_fetch_and_add(&lock->users, 1);

    while (lock->ticket != ticket)
        barrier();
    smp_mb();
}

void unlock(struct spinlock *lock) {
    smp_mb();
    (void)__sync_add_and_fetch(&lock->ticket, 1);
}
