#include "boot_allocator.h"

extern mmap_t os_mmap;

static multiboot_mmap_entry_t from_segment_to_entry(const memory_segment_t *segment) {
    multiboot_mmap_entry_t entry;

    entry.size = sizeof(multiboot_mmap_entry_t) - sizeof(uint32_t); // .size field has type uint32_t
    entry.addr = segment->begin;
    entry.len  = segment->end - segment->begin + 1;
    entry.type = 2; // we only need to reserve memory. We don't free it. Never

    return entry;
}


/**
 * I search for a free memory segment in the very beginning and right after the very end
 * of the current mmap entry. If this segment is ok, I add it to the mmap.
 * This means that according to the size of mmap_t->entries I only
 * can register up to 100 new segments. It's gonna be enough for buddy
 * allocator if it allocates all memory it needs at the start
 * using only one "malloc".
 */

memory_segment_t boot_reserve_memory(uint64_t size) {
    memory_segment_t new_segment;

    for (int i = 0; i < os_mmap.size; i++) {
        multiboot_mmap_entry_t *current = os_mmap.entries + i;

        new_segment.begin = current->addr;
        new_segment.end = new_segment.begin + size - 1;

        /**
         * we also check that this segment is inside first 4GB
         * as we only have paging on this space.
         */
        if (is_free(&new_segment) && new_segment.end <= UINT32_MAX) {
            mmap_add_entry(from_segment_to_entry(&new_segment));
            return new_segment;
        }

        if (UINT32_MAX - size + 1 < current->addr + current->len)
            continue;
        
        new_segment.begin = current->addr + current->len;
        new_segment.end = new_segment.begin + size - 1;

        if (is_free(&new_segment) && new_segment.end <= UINT32_MAX) {
            mmap_add_entry(from_segment_to_entry(&new_segment));
            return new_segment;
        }
    }

    /** 
     * I hope this wont happen but to indicate
     * that we could not find appropriate place
     * in memory we return segment with 
     * begin greater than end
     */

    new_segment.begin = 1;
    new_segment.end = 0;

    return new_segment;
}
