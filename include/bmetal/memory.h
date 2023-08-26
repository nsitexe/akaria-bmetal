/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_MEMORY_H_
#define BAREMETAL_CRT_MEMORY_H_

#include <bmetal/bmetal.h>
#include <bmetal/lock.h>
#include <bmetal/sys/types.h>

#define __PAGE_SIZE   4096

#if !defined(__ASSEMBLER__)

typedef uint64_t __paddr_t;

struct __mem_node {
	struct __spinlock lock;

	__paddr_t paddr;
	void *vaddr;
	uint64_t size;

	uint64_t page_total;
	uint64_t page_use;
	char *stat_page;
};

static inline int __mem_node_lock(struct __mem_node *m)
{
	__spinlock_lock(&m->lock);

	return 0;
}

static inline int __mem_node_unlock(struct __mem_node *m)
{
	__spinlock_unlock(&m->lock);

	return 0;
}

static inline size_t __mem_node_get_pagesize(const struct __mem_node *m)
{
	return __PAGE_SIZE;
}

static inline __paddr_t __mem_node_get_paddr(const struct __mem_node *m)
{
	return m->paddr;
}

static inline void *__mem_node_get_vaddr(const struct __mem_node *m)
{
	return m->vaddr;
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
void __mem_node_dump_heap_pages(const struct __mem_node *m);
ssize_t __mem_node_alloc_pages(struct __mem_node *m, size_t length);
int __mem_node_check_pages(struct __mem_node *m, void *start, size_t length);
int __mem_node_free_pages(struct __mem_node *m, void *start, size_t length);

int __mem_brk_lock(void);
int __mem_brk_unlock(void);
void *__mem_brk_start(void);
void *__mem_brk_end(void);
char *__mem_brk_get_cur(void);
void __mem_brk_set_cur(char *p);

int __mem_init(void);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_MEMORY_H_ */
