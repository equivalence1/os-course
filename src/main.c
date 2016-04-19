#include "file-system.h"
#include "kmem_cache.h"
#include "interrupt.h"
#include "threads.h"
#include "memory.h"
#include "serial.h"
#include "paging.h"
#include "string.h"
#include "stdio.h"
#include "misc.h"
#include "time.h"

static bool range_intersect(phys_t l0, phys_t r0, phys_t l1, phys_t r1)
{
	if (r0 <= l1)
		return false;
	if (r1 <= l0)
		return false;
	return true;
}

static void buddy_smoke_test(void)
{
	DBG_INFO("Start buddy test");
	struct page *page[10];
	int order[ARRAY_SIZE(page)];

	for (int i = 0; i != ARRAY_SIZE(page); ++i) {
		page[i] = alloc_pages(i);
		if (page[i]) {
			const phys_t begin = page_paddr(page[i]);
			const phys_t end = begin + (PAGE_SIZE << i);
			DBG_INFO("allocated [%#llx-%#llx]", begin, end - 1);
			order[i] = i;
		}
	}

	for (int i = 0; i != ARRAY_SIZE(page) - 1; ++i) {
		if (!page[i])
			break;
		for (int j = i + 1; j != ARRAY_SIZE(page); ++j) {
			if (!page[j])
				break;

			const phys_t ibegin = page_paddr(page[i]);
			const phys_t iend = ibegin + (PAGE_SIZE << order[i]);

			const phys_t jbegin = page_paddr(page[j]);
			const phys_t jend = jbegin + (PAGE_SIZE << order[j]);

			DBG_ASSERT(!range_intersect(ibegin, iend, jbegin, jend));
		}
	}

	for (int i = 0; i != ARRAY_SIZE(page); ++i) {
		if (!page[i])
			continue;

		const phys_t begin = page_paddr(page[i]);
		const phys_t end = begin + (PAGE_SIZE << i);
		DBG_INFO("freed [%#llx-%#llx]", begin, end - 1);
		free_pages(page[i], order[i]);
	}
	DBG_INFO("Buddy test finished");
}

struct intlist {
	struct list_head link;
	int data;
};

static void slab_smoke_test(void)
{
	DBG_INFO("Start SLAB test");
	struct kmem_cache *cache = KMEM_CACHE(struct intlist);
	LIST_HEAD(head);
	int i;

	for (i = 0; i != 1000000; ++i) {
		struct intlist *node = kmem_cache_alloc(cache);

		if (!node)
			break;
		node->data = i;
		list_add_tail(&node->link, &head);
	}

	DBG_INFO("Allocated %d nodes", i);

	while (!list_empty(&head)) {
		struct intlist *node = LIST_ENTRY(list_first(&head),
					struct intlist, link);

		list_del(&node->link);
		kmem_cache_free(cache, node);
	}

	kmem_cache_destroy(cache);
	DBG_INFO("SLAB test finished");
}

static int test_function(void *dummy)
{
	(void) dummy;
	return 0;
}

static void test_threading(void)
{
	DBG_INFO("Start threading test");
	for (int i = 0; i != 10000; ++i) {
		const pid_t pid = create_kthread(&test_function, 0);

		DBG_ASSERT(pid >= 0);
		wait(pid);
	}
	DBG_INFO("Threading test finished");
}

static void test_fs(void) {
    DBG_INFO("Start file system test");

    DBG_INFO("case1:\n\n");
    fd_t *fd = fs_open("/somedir/somefile", FLAG_CREATE | FLAG_WRITE | FLAG_READ);
    const char *str = "We all live\nin a yellow\nsubmarine";
    fs_write(fd, str, strlen(str) + 1);
    char *res = (char *)kmem_alloc(strlen(str) + 1);
    fs_read(fd, res, strlen(str) + 1);
    DBG_INFO("read1:\n%s\n", res);
    fs_close(fd);
    fs_read(fd, res, strlen(str) + 1);
    fd = fs_open("/somedir/somefile", FLAG_CREATE | FLAG_WRITE | FLAG_READ);
    fs_read(fd, res, strlen(str) + 1);
    DBG_INFO("read2:\n%s\n", res);
    fs_close(fd);

    DBG_INFO("case2:\n\n");

    fs_mkdir("dir1");
    fs_mkdir("/dir1");
    fs_mkdir("/dir1.5");
    fs_mkdir("///dir5");
    fs_mkdir("dir1/dir2");
    fs_mkdir("dir1/dir2/dir3");
    fs_mkdir("/dir1/dir2/dir3/");
    fs_mkdir("/dir1/dir2/dir4/");
    fs_mkdir("/");

/**
 * should be
 *
 *
 *  /
 *      somedir
 *          somefile
 *      dir1
 *          dir2
 *              dir3
 *      dir1.5
 *      dir5
 *
 *
 */

    print_full_fs();

    DBG_INFO("File system test finished");
}

static int start_kernel(void *dummy)
{
	(void) dummy;

	buddy_smoke_test();
	slab_smoke_test();
	test_threading();
    test_fs();

	return 0;
}

void main(void)
{
	setup_serial();
	setup_misc();
	setup_ints();
	setup_memory();
	setup_buddy();
	setup_paging();
	setup_alloc();
	setup_time();
	setup_threading();
    setup_fs();
	local_irq_enable();

	create_kthread(&start_kernel, 0);
	local_preempt_enable();
	idle();

	while (1);
}
