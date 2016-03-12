#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include "multiboot_structs.h"
#include "common.h"
#include "memory.h"
#include "stdio.h"

int mmap_check_ok(void);
void mmap_reserve_os(void);
void get_mmap(void);
void print_mmap_entry(const multiboot_mmap_entry_t *entry);

#endif // __MEMMAP_H__
