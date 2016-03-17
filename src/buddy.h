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
#define MAX_LEVELS 43 // 64 - 21

#define NODE_FREE      1
#define NODE_ALLOCATED 2

struct buddy_descriptor {
    char order;
    char state;
    struct list_head ptrs;
} __attribute((packed)); // we need to economy space
typedef struct buddy_descriptor buddy_descriptor_t;

int init_buddy_allocator(void);
void* buddy_allocate(uint64_t size);
void buddy_free(void *ptr);

#endif // __BUDDY_H__
