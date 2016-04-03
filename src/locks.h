#ifndef __LOCKS_H__
#define __LOCKS_H__

#define barrier() __asm__ volatile ("" : : : "memory")
#define smp_mb() __asm__ volatile ("mfence" : : : "memory")

#include <stdbool.h>
#include <stdint.h>

typedef struct spinlock {
    uint16_t users;
    uint16_t ticket;
} spinlock_t;

void lock(struct spinlock *);
void unlock(struct spinlock *);

#endif // __LOCKS_H__
