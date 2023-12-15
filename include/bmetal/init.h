/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_INIT_H_
#define BAREMETAL_CRT_INIT_H_

#include <bmetal/bmetal.h>

#define BAREMETAL_CRT_AUX_SECTION    ".auxdata"

#define DEFAULT_KERNEL_NAME    "main"

#if !defined(__ASSEMBLER__)

#define CHECK_OVERLAP(ST_A, SZ_A, ST_B, SZ_B) \
	(((ST_A) <= (ST_B)          && (ST_B)          <  (ST_A) + (SZ_A)) || \
	 ((ST_A) <  (ST_B) + (SZ_B) && (ST_B) + (SZ_B) <= (ST_A) + (SZ_A)) || \
	 ((ST_B) <= (ST_A)          && (ST_A)          <  (ST_B) + (SZ_B)) || \
	 ((ST_B) <  (ST_A) + (SZ_A) && (ST_A) + (SZ_A) <= (ST_B) + (SZ_B)))

#define define_brk(sym, size) \
	char sym[size] \
	__section(".noinit")

#define define_heap(sym, size) \
	char sym[size] \
	__aligned(__PAGE_SIZE) \
	__section(".heap")

#define define_stack(sym, size) \
	char sym[size] \
	__aligned(CONFIG_STACK_ALIGN) \
	__section(".noinit")

#define define_init_func(fn) \
	const k_init_func_t k_init_func__##fn \
	__section(".initdata") __used = fn

#define define_fini_func(fn) \
	const k_fini_func_t k_fini_func__##fn \
	__section(".finidata") __used = fn

typedef int (*k_init_func_t)(void);
typedef void (*k_fini_func_t)(void);

extern char k_stack_intr[];
extern char k_stack_idle[];
extern char k_stack_main[];

struct k_aux_data {
	uint32_t valid;
	uint64_t phent;
	uint64_t phnum;
	uint64_t phdr_size;
};

#define k_init_panic()    k_init_panic_internal(__func__, __LINE__)
void __noreturn k_init_panic_internal(const char *func, int nline);

void k_init_system(void);
void k_fini_system(void);
void k_init_leader(void);
void k_fini_leader(int status);
void k_init_child(void);
void k_fini_child(int status);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_INIT_H_ */
