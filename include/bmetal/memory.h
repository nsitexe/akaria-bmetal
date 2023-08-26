/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_MEMORY_H_
#define BAREMETAL_CRT_MEMORY_H_

#include <bmetal/bmetal.h>
#include <bmetal/lock.h>
#include <bmetal/sys/types.h>

#define __PAGE_SIZE   4096

#if !defined(__ASSEMBLER__)

struct __mem_node {
	paddr_t paddr;
	uint64_t size;

	void *vaddr;
	struct __spinlock lock;
	uint64_t page_total;
	uint64_t page_use;
	char *stat_page;
};

static inline size_t __mem_node_get_pagesize(const struct __mem_node *m)
{
	return __PAGE_SIZE;
}

static inline paddr_t __mem_node_get_paddr(const struct __mem_node *m)
{
	return m->paddr;
}

static inline uint64_t __mem_node_get_size(const struct __mem_node *m)
{
	return m->size;
}

static inline uint64_t __mem_node_get_total_pages(const struct __mem_node *m)
{
	return m->page_total;
}

static inline uint64_t __mem_node_get_used_pages(const struct __mem_node *m)
{
	return m->page_use;
}

static inline void *__mem_node_get_vaddr(const struct __mem_node *m)
{
	return m->vaddr;
}

static inline int __mem_node_get_page_stat(const struct __mem_node *m, size_t off_page)
{
	return m->stat_page[off_page];
}

int __mem_lock(void);
int __mem_unlock(void);
void *__mem_heap_area_start(void);
void *__mem_heap_area_end(void);
void __mem_dump_heap_pages(void);
int __mem_set_page_flag(size_t off_page, size_t n_page, int val);
ssize_t __mem_alloc_pages(size_t length);
int __mem_check_pages(void *start, size_t length);
int __mem_free_pages(void *start, size_t length);

struct __mem_node *__mem_node_head(void);

size_t __mem_node_get_pagesize(const struct __mem_node *m);
paddr_t __mem_node_get_paddr(const struct __mem_node *m);
void *__mem_node_get_vaddr(const struct __mem_node *m);

void __mem_node_dump_heap_pages(const struct __mem_node *m);
ssize_t __mem_node_alloc_pages(struct __mem_node *m, size_t length);
int __mem_node_check_pages(struct __mem_node *m, void *start, size_t length);
int __mem_node_free_pages(struct __mem_node *m, void *start, size_t length);

int __mem_init(void);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_MEMORY_H_ */
