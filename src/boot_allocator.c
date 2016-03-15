#include "boot_allocator.h"

extern mmap_t os_mmap;

memory_segment_t boot_reserve_memory(uint64_t size) {
    memory_segment_t new_segment;

    for (int i = 0; i < os_mmap.size; i++) {
        multiboot_mmap_entry_t *current = os_mmap.entries + i;
        if (current->addr >= size) {
            new_segment.begin = current->addr - size;
            new_segment.end = current->addr - 1;

            if (can_be_used(new_segment)) {
                reserve
                return new_segment;
            }
        }
    }
}
