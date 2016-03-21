#include "memmap.h"

extern const uint32_t mboot_info;
multiboot_info_t *mbt;
mmap_t os_mmap;

static void set_mbt() {
    mbt = (multiboot_info_t *) va(mboot_info);
}

int mmap_check_presented(void) {
    set_mbt();
    return check_bit(mbt->flags, 6);
}

static multiboot_mmap_entry_t make_os_mmap_entry() {
    extern char text_phys_begin[];
    extern char bss_phys_end[];

    uint64_t os_addr_begin = (uint64_t) text_phys_begin;
    uint64_t os_addr_end   = (uint64_t) bss_phys_end - 1;

    multiboot_mmap_entry_t os_entry;
    os_entry.size = sizeof(multiboot_mmap_entry_t) - sizeof(uint32_t); // .size field has type uint32_t
    os_entry.addr = os_addr_begin;
    os_entry.len  = os_addr_end - os_addr_begin + 1;
    os_entry.type = MMAP_ENTRY_OS_TYPE;

    return os_entry;
}

void mmap_add_entry(multiboot_mmap_entry_t entry) {
    os_mmap.entries[os_mmap.size++] = entry;
}

void mmap_reserve_os(void) {
    mmap_add_entry(make_os_mmap_entry());
}

/* this one checks that point is inside of segment */
static int point_is_inside(uint64_t point, uint64_t seg_left, uint64_t seg_right) {
    if (point >= seg_left && point <= seg_right) {
        return 1;
    } else {
        return 0;
    }
}

/* this one cheks that the first segment is inside of the second */
static int is_inside(const memory_segment_t *first, const memory_segment_t *second) {
    return (point_is_inside(first->begin, second->begin, second->end)) && 
           (point_is_inside(first->end  , second->begin, second->end));
}

static int does_intersect(const memory_segment_t *first,
        const memory_segment_t *second) {
    if (    point_is_inside(first->begin , second->begin, second->end) ||
            point_is_inside(first->end   , second->begin, second->end) ||
            point_is_inside(second->begin, first->begin , first->end)) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * segment can be used if it lies inside some free block
 * and doesn't itersect with any already reserved block
 */
int is_free(const memory_segment_t *segment) {
    int lay_in_free = 0;
    int intersect_with_reserved = 0;

    for (int i = 0; i < os_mmap.size; i++) {
        memory_segment_t current;
        current.begin = os_mmap.entries[i].addr;
        current.end = os_mmap.entries[i].addr + os_mmap.entries[i].len - 1;

        if (is_inside(segment, &current) && os_mmap.entries[i].type == 1)
            lay_in_free = 1;

        if (does_intersect(segment, &current) && os_mmap.entries[i].type != 1) {
            intersect_with_reserved = 1;
            break;
        }
    }

    return (lay_in_free == 1) && (intersect_with_reserved == 0);
}

memory_segment_t get_memory_range(void) {
    memory_segment_t segment;
    segment.begin = UINT64_MAX;
    segment.end   = 0;

    for (int i = 0; i < os_mmap.size; i++) {
        segment.begin = MIN(segment.begin, os_mmap.entries[i].addr);
        segment.end   = MAX(segment.end, os_mmap.entries[i].addr + os_mmap.entries[i].len - 1);
    }

    return segment;
}

void get_mmap(void) {
    os_mmap.size = 0;
    /*
     * I cast all to uint64_t only to get rid of annoying warning in YouCompleteMe
     */
    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *) ((uint64_t) mbt->mmap_addr);

    while ((uint64_t) mmap < mbt->mmap_addr + mbt->mmap_length) {
        os_mmap.entries[os_mmap.size++] = *mmap;
        mmap = (multiboot_mmap_entry_t *) ((uint64_t)mmap + mmap->size + sizeof(mmap->size));
    }
}

void print_mmap_entry(const multiboot_mmap_entry_t *entry) {
    unsigned long long mem_start = entry->addr;
    unsigned long long mem_end   = entry->addr + entry->len - 1;
    int mem_type  = entry->type;

    printf("memory range: %#llx-%#llx, type %d\n", mem_start, mem_end, mem_type);
}