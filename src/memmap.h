#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include "multiboot_structs.h"
#include "common.h"
#include "memory.h"
#include "stdio.h"

// type 1 means that section is free to use. Anything else means that it's reserved.
// We want to reserve our OS space. I choosed 3 only to idicate that this is OS
#define MMAP_ENTRY_OS_TYPE 3 

int mmap_check_ok(void);

void mmap_add_entry(multiboot_mmap_entry_t entry);
void mmap_reserve_os(void);
void get_mmap(void);
void print_mmap_entry(const multiboot_mmap_entry_t *entry);

#endif // __MEMMAP_H__
