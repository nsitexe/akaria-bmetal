/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/init.h>
#include <bmetal/lock.h>
#include <bmetal/memory.h>
#include <bmetal/printk.h>
#include <bmetal/sys/mman.h>
#include <bmetal/sys/types.h>

#if (CONFIG_HEAP_SIZE % __PAGE_SIZE) != 0
#  error Invalid heap size. It should be aligned page size. \
         Please check configs about HEAP_SIZE.
#endif

static struct __mem_node node_head;

static struct __spinlock lock_mem;
static define_heap(heap_head_area, CONFIG_HEAP_SIZE);
static char heap_head_stat[CONFIG_HEAP_SIZE / __PAGE_SIZE];
static int dbg_heap_num = 1;

struct __mem_node *__mem_node_head(void)
{
	return &node_head;
}

static size_t __mem_node_count_cont_pages(const struct __mem_node *m, size_t off_page, size_t lim_page)
{
	size_t r = 1;

	for (size_t i = off_page; i < m->page_total; i++, r++) {
		if (m->stat_page[i] || r >= lim_page) {
			break;
		}
	}

	return r;
}

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
	const struct __mem_node *m = __mem_node_head();

	return m->vaddr;
}

void *__mem_heap_area_end(void)
{
	const struct __mem_node *m = __mem_node_head();

	return m->vaddr + m->size;
}

void __mem_dump_heap_pages(void)
{
	__mem_node_dump_heap_pages(__mem_node_head());
}

ssize_t __mem_alloc_pages(size_t length)
{
	return __mem_node_alloc_pages(__mem_node_head(), length);
}

int __mem_check_pages(void *start, size_t length)
{
	return __mem_node_check_pages(__mem_node_head(), start, length);
}

int __mem_free_pages(void *start, size_t length)
{
	return __mem_node_free_pages(__mem_node_head(), start, length);
}

static int __mem_node_set_page_flag(struct __mem_node *m, size_t off_page, size_t n_page, int val)
{
	for (size_t i = off_page; i < off_page + n_page; i++) {
		m->stat_page[i] = val;
	}

	return 0;
}

void __mem_node_dump_heap_pages(const struct __mem_node *m)
{
#ifdef CONFIG_DEBUG_HEAP
	pri_dbg("dump_heap_pages:\n");
	for (size_t i = 0; i < m->page_total; i++) {
		if (i % 16 == 0) {
			pri_dbg("  %4d: ", (int)i);
		}
		pri_dbg("%2x ", m->stat_page[i]);
		if (i % 16 == 15) {
			pri_dbg("\n");
		}
	}
	pri_dbg("\n");
#endif
}

ssize_t __mem_node_alloc_pages(struct __mem_node *m, size_t length)
{
	if (!m || length == 0) {
		return -EINVAL;
	}

	size_t pgsize = __mem_node_get_pagesize(m);
	size_t req_page = (length + pgsize - 1) / pgsize;
	size_t i = 0;

	if (req_page > m->page_total - m->page_use) {
		pri_warn("alloc_pages: no more pages (req:%d, free:%d)\n",
			(int)req_page, (int)(m->page_total - m->page_use));
		return -ENOMEM;
	}

	while (i < m->page_total) {
		if (m->stat_page[i]) {
			i++;
			continue;
		}

		size_t cnt_page = __mem_node_count_cont_pages(m, i, req_page);
		if (cnt_page >= req_page) {
			__mem_node_set_page_flag(m, i, req_page, dbg_heap_num);
			m->page_use += req_page;

			dbg_heap_num += 1;
			if ((dbg_heap_num & 0xff) == 0) {
				dbg_heap_num += 1;
			}

			pri_info("alloc_pages: heap:%d, page:%d-%d (%d KB)\n", dbg_heap_num,
				(int)i, (int)(i + req_page - 1), (int)(req_page * pgsize / 1024));
			__mem_node_dump_heap_pages(m);
			return i;
		}

		i += cnt_page;
	}

	return -1;
}

int __mem_node_check_pages(struct __mem_node *m, void *start, size_t length)
{
	if (!m || length == 0) {
		return -EINVAL;
	}

	size_t pgsize = __mem_node_get_pagesize(m);
	
	if (start < m->vaddr || (m->vaddr + m->size <= start)) {
		pri_warn("check_pages: out of range 0x%p.\n", start);
		return -EINVAL;
	}

	uintptr_t off_addr = start - m->vaddr;
	size_t off_page = off_addr / pgsize;
	size_t n_page = length / pgsize;
	int failed = 0;

	if (off_addr & (pgsize - 1)) {
		n_page += 1;
	}

	for (size_t i = off_page; i < off_page + n_page; i++) {
		if (!m->stat_page[i]) {
			pri_warn("check_pages: not allocated at %d.\n", (int)i);
			failed = 1;
		}
	}
	if (failed) {
		__mem_node_dump_heap_pages(m);

		return -EBADF;
	}

	return 0;
}

int __mem_node_free_pages(struct __mem_node *m, void *start, size_t length)
{
	if (!m || length == 0) {
		return -EINVAL;
	}
	
	size_t pgsize = __mem_node_get_pagesize(m);

	if (start < m->vaddr || (m->vaddr + m->size <= start)) {
		pri_warn("free_pages: out of range 0x%p.\n", start);
		return -EINVAL;
	}

	uintptr_t off_addr = start - m->vaddr;
	size_t off_page = off_addr / pgsize;
	size_t n_page = length / pgsize;
	int need_dump = 0;

	if (off_addr & (pgsize - 1)) {
		n_page += 1;
	}

	for (size_t i = off_page; i < off_page + n_page; i++) {
		if (!m->stat_page[i]) {
			pri_warn("free_pages: double free at %d.\n", (int)i);
			need_dump = 1;
		}
	}
	if (need_dump) {
		__mem_dump_heap_pages();
	}

	__mem_node_set_page_flag(m, off_page, n_page, 0);
	m->page_use -= n_page;

	pri_info("free_pages: page:%d-%d (%d KB)\n", (int)off_page,
		(int)(off_page + n_page - 1), (int)(n_page * pgsize / 1024));

	return 0;
}

int __mem_init(void)
{
	struct __mem_node *mnode = __mem_node_head();
	int res = 0;

	mnode->paddr = (uintptr_t)heap_head_area;
	mnode->size = CONFIG_HEAP_SIZE;

	mnode->vaddr = heap_head_area;
	mnode->page_total = CONFIG_HEAP_SIZE / __PAGE_SIZE;
	mnode->page_use = 0;
	mnode->stat_page = heap_head_stat;

	return res;
}
