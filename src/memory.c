#include "memory.h"

uint64_t get_bits(uint64_t x, int from, int how_many) {
    return (x >> from) & ((1L << how_many) - 1);
}
