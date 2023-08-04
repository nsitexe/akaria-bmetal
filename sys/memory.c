/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/init.h>
#include <bmetal/lock.h>
#include <bmetal/memory.h>
#include <bmetal/printk.h>
#include <bmetal/sys/mman.h>
#include <bmetal/sys/types.h>

#if defined(CONFIG_HEAP) && ((CONFIG_HEAP_SIZE % __PAGE_SIZE) != 0)
#  error Invalid heap size. It should be aligned page size. \
         Please check configs about HEAP_SIZE.
#endif

#ifdef CONFIG_HEAP

static struct __spinlock lock_mem;
static define_heap(heap_area, CONFIG_HEAP_SIZE);
static char heap_used[CONFIG_HEAP_SIZE / __PAGE_SIZE];
static int dbg_heap_num = 1;

int __mem_lock(void)
{
	__spinlock_lock(&lock_mem);

	return 0;
}

int __mem_unlock(void)
{
	__spinlock_unlock(&lock_mem);

	return 0;
}

void *__mem_heap_area_start(void)
{
	return &heap_area[0];
}

void *__mem_heap_area_end(void)
{
	return &heap_area[0] + CONFIG_HEAP_SIZE;
}

void __mem_dump_heap_pages(void)
{
#ifdef CONFIG_DEBUG_HEAP
	pri_dbg("dump_heap_pages:\n");
	for (size_t i = 0; i < ARRAY_OF(heap_used); i++) {
		if (i % 16 == 0) {
			pri_dbg("  %4d: ", (int)i);
		}
		pri_dbg("%2x ", heap_used[i]);
		if (i % 16 == 15) {
			pri_dbg("\n");
		}
	}
	pri_dbg("\n");
#endif
}

static size_t __mem_get_cont_pages_num(size_t off_page, size_t lim_page)
{
	size_t r = 1;

	for (size_t i = off_page; i < ARRAY_OF(heap_used); i++, r++) {
		if (heap_used[i] || r >= lim_page) {
			break;
		}
	}

	return r;
}

int __mem_set_page_flag(size_t off_page, size_t n_page, int val)
{
	for (size_t i = off_page; i < off_page + n_page; i++) {
		heap_used[i] = val;
	}

	return 0;
}

ssize_t __mem_alloc_pages(size_t len)
{
	size_t size_page = (len + __PAGE_SIZE - 1) / __PAGE_SIZE;
	size_t i = 0;

	while (i < ARRAY_OF(heap_used)) {
		if (heap_used[i]) {
			i++;
			continue;
		}

		size_t n_page = __mem_get_cont_pages_num(i, size_page);
		if (n_page >= size_page) {
			__mem_set_page_flag(i, size_page, dbg_heap_num);
			dbg_heap_num += 1;
			if ((dbg_heap_num & 0xff) == 0) {
				dbg_heap_num += 1;
			}

			pri_info("alloc_pages: heap:%d, page:%d-%d (%d KB)\n", dbg_heap_num,
				(int)i, (int)(i + size_page - 1), (int)(size_page * __PAGE_SIZE / 1024));
			__mem_dump_heap_pages();
			return i;
		}

		i += n_page;
	}

	return -1;
}

int __mem_check_pages(void *start, size_t length)
{
	uintptr_t off_addr = start - __mem_heap_area_start();
	size_t off_page = off_addr / __PAGE_SIZE;
	size_t n_page = length / __PAGE_SIZE;
	int failed = 0;

	if (off_addr & (__PAGE_SIZE - 1)) {
		n_page += 1;
	}

	for (size_t i = off_page; i < off_page + n_page; i++) {
		if (!heap_used[i]) {
			pri_warn("check_pages: not allocated at %d.\n", (int)i);
			failed = 1;
		}
	}
	if (failed) {
		__mem_dump_heap_pages();

		return -EBADF;
	}

	return 0;
}

int __mem_free_pages(void *start, size_t length)
{
	uintptr_t off_addr = start - __mem_heap_area_start();
	size_t off_page = off_addr / __PAGE_SIZE;
	size_t n_page = length / __PAGE_SIZE;
	int need_dump = 0;

	if (off_addr & (__PAGE_SIZE - 1)) {
		n_page += 1;
	}

	for (size_t i = off_page; i < off_page + n_page; i++) {
		if (!heap_used[i]) {
			pri_warn("free_pages: double free at %d.\n", (int)i);
			need_dump = 1;
		}
	}
	if (need_dump) {
		__mem_dump_heap_pages();
	}

	__mem_set_page_flag(off_page, n_page, 0);

	pri_info("free_pages: page:%d-%d (%d KB)\n", (int)off_page,
		(int)(off_page + n_page - 1), (int)(n_page * __PAGE_SIZE / 1024));

	return 0;
}

#endif /* CONFIG_HEAP */
