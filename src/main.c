#include "interrupt.h"
#include "serial.h"
#include "stdio.h"
#include "time.h"
#include "memmap.h"
#include "common.h"

void main(void)
{
    setup_serial();
    setup_ints();
    setup_time();

    local_irq_enable();

    if (mmap_check_ok()) {
        get_mmap();
        mmap_reserve_os();

        extern mmap_t os_mmap;
        printf(ANSI_COLOR_YELLOW);
        for_each_macro(os_mmap.entries, os_mmap.size, print_mmap_entry);
        printf(ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_RED "multiboot did not provide memory map, aborting." ANSI_COLOR_RESET "\n");
        goto kernel_end;
    }

kernel_end:
    while (1) {
        cpu_hlt();
    }
}
