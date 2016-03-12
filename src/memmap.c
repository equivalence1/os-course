#include "memmap.h"

extern const uint32_t mboot_info;
multiboot_info_t *mbt;
mmap_t os_mmap;

static void set_mbt() {
    mbt = (multiboot_info_t *) va(mboot_info);
}

int mmap_check_ok(void) {
    set_mbt();
    return check_bit(mbt->flags, 6);
}

static multiboot_mmap_entry_t make_os_mmap_entry() {
    extern char text_phys_begin[];
    extern char bss_phys_end[];

    uint64_t os_addr_begin = (uint64_t) text_phys_begin;
    uint64_t os_addr_end   = (uint64_t) bss_phys_end;
    
    multiboot_mmap_entry_t os_entry;
    os_entry.size = 20;
    os_entry.addr = os_addr_begin;
    os_entry.len  = os_addr_end - os_addr_begin + 1;
    os_entry.type = 2;

    return os_entry;
}

static inline int is_inside(multiboot_mmap_entry_t *a, multiboot_mmap_entry_t *b) {
    return (b->addr <= a->addr && b->addr + b->len >= a->addr);
}

void mmap_reserve_os(void) {
    multiboot_mmap_entry_t os_entry = make_os_mmap_entry();
    os_mmap.entries[os_mmap.size++] = os_entry;
}

void get_mmap(void) {
    os_mmap.size = 0;

    /*
     * I cast all to uint64_t only to get rid of annoying warning in YouCompleteMe
     */
    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *) ((uint64_t) mbt->mmap_addr);

    while ((uint64_t) mmap < mbt->mmap_addr + mbt->mmap_length) {
        os_mmap.entries[os_mmap.size++] = *mmap;
        mmap = (multiboot_mmap_entry_t *) ((uint64_t)mmap + mmap->size + sizeof(uint32_t));
    }
}

void print_mmap_entry(const multiboot_mmap_entry_t *entry) {
    uint64_t mem_start = entry->addr;
    uint64_t mem_end   = entry->addr + entry->len - 1;
    uint32_t mem_type  = entry->type;

    printf("memory range: 0x%x-0x%x, type %d\n", mem_start, mem_end, mem_type);
}
