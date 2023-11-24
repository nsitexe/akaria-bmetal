/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_MEMORY_H_
#define BAREMETAL_CRT_MEMORY_H_

#include <bmetal/bmetal.h>
#include <bmetal/lock.h>
#include <bmetal/sys/types.h>

#define __PAGE_SIZE   4096

#if !defined(__ASSEMBLER__)

typedef uint64_t k_paddr_t;

struct k_mem_node {
	struct k_spinlock lock;

	k_paddr_t paddr;
	void *vaddr;
	uint64_t size;

	uint64_t page_total;
	uint64_t page_use;
	char *stat_page;
};

static inline int k_mem_node_lock(struct k_mem_node *m)
{
	k_spinlock_lock(&m->lock);

	return 0;
}

static inline int k_mem_node_unlock(struct k_mem_node *m)
{
	k_spinlock_unlock(&m->lock);

	return 0;
}

static inline size_t k_mem_node_get_pagesize(const struct k_mem_node *m)
{
	return __PAGE_SIZE;
}

static inline k_paddr_t k_mem_node_get_paddr(const struct k_mem_node *m)
{
	return m->paddr;
}

static inline void *k_mem_node_get_vaddr(const struct k_mem_node *m)
{
	return m->vaddr;
}

static inline uint64_t k_mem_node_get_size(const struct k_mem_node *m)
{
	return m->size;
}

static inline uint64_t k_mem_node_get_total_pages(const struct k_mem_node *m)
{
	return m->page_total;
}

static inline uint64_t k_mem_node_get_used_pages(const struct k_mem_node *m)
{
	return m->page_use;
}

static inline int k_mem_node_get_page_stat(const struct k_mem_node *m, size_t off_page)
{
	return m->stat_page[off_page];
}

void *k_mem_heap_area_start(void);
void *k_mem_heap_area_end(void);
void k_mem_dump_heap_pages(void);
int k_mem_set_page_flag(size_t off_page, size_t n_page, int val);
ssize_t k_mem_alloc_pages(size_t length);
int k_mem_check_pages(void *start, size_t length);
int k_mem_free_pages(void *start, size_t length);

struct k_mem_node *k_mem_node_head(void);
void k_mem_node_dump_heap_pages(const struct k_mem_node *m);
ssize_t k_mem_node_alloc_pages_nolock(struct k_mem_node *m, size_t length);
int k_mem_node_check_pages_nolock(struct k_mem_node *m, void *start, size_t length);
int k_mem_node_free_pages_nolock(struct k_mem_node *m, void *start, size_t length);

static inline ssize_t k_mem_node_alloc_pages(struct k_mem_node *m, size_t length)
{
	ssize_t r;

	k_mem_node_lock(m);
	r = k_mem_node_alloc_pages_nolock(m, length);
	k_mem_node_unlock(m);

	return r;
}

static inline int k_mem_node_check_pages(struct k_mem_node *m, void *start, size_t length)
{
	int r;

	k_mem_node_lock(m);
	r = k_mem_node_check_pages_nolock(m, start, length);
	k_mem_node_unlock(m);

	return r;

}

static inline int k_mem_node_free_pages(struct k_mem_node *m, void *start, size_t length)
{
	int r;

	k_mem_node_lock(m);
	r = k_mem_node_free_pages_nolock(m, start, length);
	k_mem_node_unlock(m);

	return r;
}

int k_mem_brk_lock(void);
int k_mem_brk_unlock(void);
void *k_mem_brk_start(void);
void *k_mem_brk_end(void);
char *k_mem_brk_get_cur(void);
void k_mem_brk_set_cur(char *p);

int k_mem_init(void);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_MEMORY_H_ */
