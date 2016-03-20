#include "interrupt.h"
#include "serial.h"
#include "stdio.h"
#include "time.h"
#include "memmap.h"
#include "common.h"
#include "buddy.h"
#include "paging.h"
#include "test.h"

void print_memory_map(void) {
    extern mmap_t os_mmap;
    printf(ANSI_COLOR_YELLOW);
    for_each_macro(os_mmap.entries, os_mmap.size, print_mmap_entry);
    printf(ANSI_COLOR_RESET);
}

void main(void)
{
    setup_serial();
    setup_ints();
    setup_time();

    local_irq_enable();

    if (mmap_check_presented()) {
        get_mmap();
        mmap_reserve_os();

        print_memory_map();

        if (init_buddy_allocator() < 0) {
            printf(ANSI_COLOR_RED "could not init buddy allocator." ANSI_COLOR_RESET "\n");
            goto kernel_end;
        }

        if (make_new_paging() < 0)
            printf(ANSI_COLOR_RED "could not set new paging" ANSI_COLOR_RESET "\n");
        else {
            activate_new_paging();
            printf(ANSI_COLOR_GREEN "new paging activated!" ANSI_COLOR_RESET "\n");
        }

        run_test();
    } else {
        printf(ANSI_COLOR_RED "multiboot did not provide memory map, aborting." ANSI_COLOR_RESET "\n");
        goto kernel_end;
    }

kernel_end:
    while (1) {
        cpu_hlt();
    }
}
