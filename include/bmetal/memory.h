/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_MEMORY_H_
#define BAREMETAL_CRT_MEMORY_H_

#include <bmetal/bmetal.h>
#include <bmetal/sys/types.h>

#define __PAGE_SIZE   4096

#if !defined(__ASSEMBLER__)

int __mem_lock(void);
int __mem_unlock(void);
void *__mem_heap_area_start(void);
void *__mem_heap_area_end(void);
void __mem_dump_heap_pages(void);
int __mem_set_page_flag(size_t off_page, size_t n_page, int val);
ssize_t __mem_alloc_pages(size_t len);
int __mem_check_pages(void *start, size_t length);
int __mem_free_pages(void *start, size_t length);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_MEMORY_H_ */
