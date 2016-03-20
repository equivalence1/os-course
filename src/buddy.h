#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <stdint.h>

#include "list.h"
#include "memmap.h"
#include "common.h"
#include "memory.h"
#include "boot_allocator.h"
#include "multiboot_structs.h"

#define PAGE (1L << 21) // 2 MiB
/**
 * IA-32E paging supports only 52-bit phys. addresses maximum.
 * this means I can't have order > log2 ((1 << 53) / PAGE) = 31
 */
#define MAX_LEVELS 31

#define NODE_FREE      1
#define NODE_ALLOCATED 2

struct buddy_descriptor {
    char order;
    char state;
    struct list_head ptrs;
} __attribute__((packed)); // we need to economy space
typedef struct buddy_descriptor buddy_descriptor_t;

int init_buddy_allocator(void);
void* buddy_allocate(uint64_t size);
void buddy_free(void *ptr);
uint64_t get_pages_count(void);

#endif // __BUDDY_H__
