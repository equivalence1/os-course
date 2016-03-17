#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include "multiboot_structs.h"
#include "memory.h"
#include "stdio.h"
#include "kernel.h"
#include "common.h"

// type 1 means that section is free to use. Anything else means that it's reserved.
// We want to reserve our OS space. I choosed 3 only to idicate that this is OS
#define MMAP_ENTRY_OS_TYPE 3 

int mmap_check_presented(void);

void mmap_add_entry(multiboot_mmap_entry_t entry);
void mmap_reserve_os(void);
void get_mmap(void);
void print_mmap_entry(const multiboot_mmap_entry_t *entry);
memory_segment_t get_memory_range(void);

int is_free(const memory_segment_t *segment);

#endif // __MEMMAP_H__
