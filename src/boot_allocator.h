#ifndef __BOOT_ALLOCATOR_H__
#define __BOOT_ALLOCATOR_H__

#include <stdint.h>
#include "memory.h"
#include "common.h"
#include "memmap.h"
#include "multiboot_structs.h"

memory_segment_t boot_reserve_memory(uint64_t size);

#endif // __BOOT_ALLOCATOR_H__
