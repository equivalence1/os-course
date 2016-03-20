#include "test.h"

// FOR DEBUG ONLY!
void run_test() {
#ifndef DEBUG
    return;
#endif //DEBUG
    printf(ANSI_COLOR_GREEN "initialized" ANSI_COLOR_RESET "\n");

    char *ptr[100];

    int n = 10;
    int m = 5;
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < 2 * m; i++) {
            ptr[i] = (char *)buddy_allocate(2097152 * (1L << (i % m)));
            printf(ANSI_COLOR_BLUE "ptr: %#llx\n" ANSI_COLOR_RESET, ptr[i]);
        }
        for (int i = 0; i < 2 * m; i++) {
            buddy_free(ptr[i]);
        }
    }
}
// FOR DEBUG ONLY!
