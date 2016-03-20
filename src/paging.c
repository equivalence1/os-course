#include "paging.h"

static const uint64_t kTableSpace = TABLE_SIZE * sizeof(pte_t);

/**
 * level 1 -- our fragments;
 * level 2 -- PDE level;
 * level 3 -- PDPTE level;
 * level 4 -- PML4E level;
 */
static pte_t *pml4_addr;
static uint64_t  tables_count[4];
static uint64_t entryes_count[4];

/**
 * it finds number of table on the previus level which
 * our entry (stored in ptr) should point to.
 */
static inline uint64_t get_table_num(uint64_t va_start, uint64_t ptr, int shift) {
    return (linear(ptr) >> shift) - (linear(va_start) >> shift);
}

                 // 4K alligned         4K alligned
int create_paging(uint64_t va_start, uint64_t pa_start, uint64_t len) {
    va_start = linear(va_start); // I do this to prevent overflowing in for loops and get_lvl_i
    uint64_t n_pages = (len + (FRAME_SIZE - 1)) / FRAME_SIZE;

    entryes_count[1] = n_pages;
    tables_count[1]  = n_pages;

    uint64_t all_tables_count = 0;

    for (int i = 2; i < 4; i++) {
        entryes_count[i] = tables_count[i - 1];
        tables_count[i]  = (entryes_count[i] + (TABLE_SIZE - 1)) / TABLE_SIZE;

        all_tables_count += tables_count[i];
    }

    char *first_addr     = (char *)buddy_allocate(all_tables_count * kTableSpace);
    pte_t *l2_first_addr = (pte_t *)first_addr;
    pte_t *l3_first_addr = (pte_t *)((char *)l2_first_addr + kTableSpace * tables_count[2]);

    for (uint64_t i = 0; i < all_tables_count * TABLE_SIZE; i++) {
        pte_t *entry = (pte_t *)(first_addr + i * sizeof(pte_t));
        *entry = 0;
    }

    if (first_addr == NULL)
        return -1;

    for (uint64_t ptr = va_start; ptr < va_start + len; ptr += FRAME_SIZE) {
        uint64_t index_pa = get_table_num(va_start, ptr, 21);
        uint64_t index_va = index_pa + pml2_i(va_start);

        *(l2_first_addr + index_va)  = PTE_LARGE | PTE_PRESENT | PTE_WRITE;
        *(l2_first_addr + index_va) |= pa_start + index_pa * FRAME_SIZE;
    }
    
    for (uint64_t ptr = va_start; ptr < va_start + len; ptr += FRAME_SIZE) {
        uint64_t index_pa = get_table_num(va_start, ptr, 21 + 9);
        uint64_t index_va = index_pa + pml3_i(va_start);

        *(l3_first_addr + index_va)  = PTE_PRESENT | PTE_WRITE;
        *(l3_first_addr + index_va) |= pa(l2_first_addr + index_pa * TABLE_SIZE);
    }

    for (uint64_t ptr = va_start; ptr < va_start + len; ptr += FRAME_SIZE) {
        uint64_t index_va = pml4_i(ptr);
        uint64_t index_pa = index_va - pml4_i(ptr);

        *(pml4_addr + index_va)  = PTE_PRESENT | PTE_WRITE;
        *(pml4_addr + index_va) |= pa(l3_first_addr + index_pa * TABLE_SIZE);
    }

    return 0;
}

/** 
 * I use 2MiB only for pml4. This is bad
 * but much easyer to implement.
 */
static int create_pml4(void) {
    pml4_addr = (pte_t *)buddy_allocate(kTableSpace);
    if (pml4_addr == NULL)
        return -1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        *(pml4_addr + i) = 0;
    }

    return 0;
}

int make_new_paging(void) {
    if (create_pml4() < 0)
        return -1;
    if (create_paging(KERNEL_BASE , 0, (1L << 31)) < 0)
        return -1;
    memory_segment_t mem_range = get_memory_range();
    if (create_paging(HIGH_BASE, 0, mem_range.end - mem_range.begin + 1) < 0)
        return -1;
    return 0;
}

void activate_new_paging() {
    store_pml4(pa(pml4_addr));
    flush_tlb();
}
