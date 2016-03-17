#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define for_each_macro(arr, size, f) \
    for (int i = 0; i < (int) size; i++) { \
        f(&arr[i]); \
    }

/** 
 * I don't know why but I'm experiensing some
 * trougles in uint64_t-uint64_t division
 */
static inline uint64_t divide_two(uint64_t x, uint64_t y) {
    return (uint64_t)((unsigned long long)(x) / (unsigned long long)(y));
}

static inline uint64_t get_bits(uint64_t x, int from, int how_many) {
    return (x >> from) & ((1L << how_many) - 1);
}

static inline int check_bit(uint64_t x, int bit) {
    return get_bits(x, bit, 1);
}

static inline void cpu_hlt()
{ __asm__ volatile ("hlt"); }

#endif // __COMMON_H__
