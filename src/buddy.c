#include "buddy.h"

/**
 * CONTRACT: if buddy descriptor is a "group leader"
 * (it means it is a first descriptor in buddy node)
 * then it has it's order setted. If it's not in any list
 * then it can be not a "group leader" or it can be just
 * allocated. If it's not a "group leader" it's order setted
 * to 0 (just for better representativity), if it's allocated
 * then it's order left unchanged, so user don't need to keep
 * the order of allocated block. We return him an address to
 * the start of the allocated segment and keep order of buddy
 * descriptor unchagend. When user frees this space he gives us
 * this address and we just look at unchanged order.
 */


/**
 * it contains _physical_ addresses!
 */
static memory_segment_t buddy_space;
static uint64_t n_buddyes;
static struct list_head list[MAX_LEVELS];
static char *first_addr;

/**
 * returns the order number of descripor among all descriptors
 */
static inline uint64_t get_number_in_level(const buddy_descriptor_t *desc, int level_n) {
    return divide_two(pa(desc) - buddy_space.begin, sizeof(buddy_descriptor_t) * (1 << level_n));
}

static inline buddy_descriptor_t* get_my_buddy(const buddy_descriptor_t *me) {
    uint64_t my_number    = get_number_in_level(me, me->order);
    uint64_t buddy_number = my_number ^ 1;

    return (buddy_descriptor_t *) (first_addr + (buddy_number * (1L << me->order)));
}
/*
static int are_buddyes(const buddy_descriptor_t *first, 
        const buddy_descriptor_t *second) {
    return (get_my_buddy(first) == second);
}
*/
static inline int can_merge(const buddy_descriptor_t *first,
        const buddy_descriptor_t *second) {
    return (first->state == NODE_FREE && second->state == NODE_FREE
            && first->order == second->order);
}

static inline void swap(buddy_descriptor_t **first,
        buddy_descriptor_t **second) {
    buddy_descriptor_t *tmp = *first;
    *first = *second;
    *second = tmp;
}

static void merge(buddy_descriptor_t *first,
        buddy_descriptor_t *second) {
    if (first > second)
        swap(&first, &second);

    list_del(&first->ptrs);
    list_del(&second->ptrs);

    first->order++;
    second->order = 0;

    list_add_tail(&first->ptrs, list + first->order);
}

/**
 * this desc needs to be "group leader"
 */
static void split(buddy_descriptor_t *desc) {
    list_del(&desc->ptrs);
    desc->order -= 1;
    list_add_tail(&desc->ptrs, list + desc->order);

    buddy_descriptor_t *desc_buddy = get_my_buddy(desc);
    desc_buddy->order = desc->order;
    list_add_tail(&desc_buddy->ptrs, list + desc_buddy->order);
}

static void build_lists(void) {
    for (int i = 0; i < MAX_LEVELS; i++) {
        printf("new level: %d\n", i);
        if (list_empty(list + i))
            break;

        struct list_head *current_head = list[i].next;
        while (current_head != list + i) {
            buddy_descriptor_t *current = 
                LIST_ENTRY(current_head, buddy_descriptor_t, ptrs);
            buddy_descriptor_t *current_buddy = get_my_buddy(current);

            if (can_merge(current, current_buddy)) {
                /**
                 * current_head->prev can not point on current_buddy
                 * because we would merge them already on the previous
                 * step in that case
                 */
                struct list_head *prev_head = current_head->prev;
                merge(current, current_buddy);
                current_head = prev_head->next;
                printf("merged\n");
            } else {
                current_head = current_head->next;
            }
            printf("current_head: %#llx, list: %#llx\n", current_head, list + i);
        }
    }
}

static void init_buddy_descriptors(void) {
    for (int i = 0; i < MAX_LEVELS; i++) {
        list_init(list + i);
    }

    /**
     * initially all descriptors have order = 0
     */
    first_addr = (char *)(va(buddy_space.begin));
    buddy_descriptor_t *desc = (buddy_descriptor_t *)first_addr;

    for (uint64_t i = 0; i < n_buddyes; i++) {
        desc->order     = 0;
        desc->state     = NODE_FREE;

        list_add_tail(&desc->ptrs, list + 0);

        desc += 1;
    }
}

/**
 * Mapping on _all_ memory from 0 to MAX
 * Even if this segment does not exist
 */
int init_buddy_allocator(void) {
    memory_segment_t all_mem = get_memory_range();

    uint64_t mem_size = all_mem.end - all_mem.begin;

    n_buddyes = divide_two(mem_size, PAGE);
    if (mem_size % PAGE != 0)
        n_buddyes += 1;

    buddy_space = boot_reserve_memory(n_buddyes * sizeof(buddy_descriptor_t));
    
    if (buddy_space.begin > buddy_space.end)
        return -1;

    printf("init_desc\n");
    init_buddy_descriptors();
    printf("build_list\n");
    build_lists();
    printf("done\n");

    return 0;
}

static inline int get_min_order_to_cover(uint64_t size) {
    int order = 0;

    while ((uint64_t)(1 << order) < size) {
        order += 1;
    }

    return order;
}

void* buddy_allocate(uint64_t size) {
    int min_order = get_min_order_to_cover(size);
    int cur_order = min_order;

    while (list_empty(list + cur_order)) {
        cur_order++;
    }

    while (cur_order != min_order) {
        buddy_descriptor_t *desc = 
            CONTAINER_OF(list_first(list + cur_order), buddy_descriptor_t, ptrs);
        split(desc);
        cur_order--;
    }

    buddy_descriptor_t *allocated =
        CONTAINER_OF(list_first(list + min_order), buddy_descriptor_t, ptrs);
    allocated->state = NODE_ALLOCATED;

    int in_level_number = get_number_in_level(allocated, allocated->order);

    return va(buddy_space.begin + ((uint64_t)PAGE * (1L << allocated->order) * in_level_number));
}
