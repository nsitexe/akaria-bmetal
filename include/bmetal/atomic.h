/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ATOMIC_H_
#define BAREMETAL_CRT_ATOMIC_H_

#include <bmetal/bmetal.h>

#if !defined(CONFIG_ATOMIC) && (CONFIG_NUM_CORES > 1)
#  error Cannot enable SMP without ATOMIC feature. \
         Please check configs about ATOMIC and NUM_CORES.
#endif

#if !defined(__ASSEMBLER__)

#if CONFIG_ATOMIC
#include <stdatomic.h>
#endif

#if defined(CONFIG_ATOMIC)

typedef atomic_int __atomic_int;
typedef atomic_long __atomic_long;

#define __aload_add_explicit(obj, ord)    atomic_load_add_explicit(obj, ord)
#define __aload_add(obj)                  atomic_load_add(obj)
#define __astore_explicit(obj, arg, ord)    atomic_store_explicit(obj, arg, ord)
#define __astore(obj, arg)                  atomic_store(obj, arg)
#define __aexchange_explicit(obj, arg, ord)    atomic_exchange_explicit(obj, arg, ord)
#define __aexchange(obj, arg)                  atomic_exchange(obj, arg)
#define __afetch_add_explicit(obj, arg, ord)    atomic_fetch_add_explicit(obj, arg, ord)
#define __afetch_add(obj, arg)                  atomic_fetch_add(obj, arg)

#else /* CONFIG_ATOMIC */

typedef volatile int __atomic_int;
typedef volatile long __atomic_long;

#define __aload_explicit(obj, arg, ord)    __aload(obj)
#define __astore_explicit(obj, arg, ord)    __astore(obj, arg)
#define __aexchange_explicit(obj, arg, ord)    __aexchange(obj, arg)
#define __afetch_add_explicit(obj, arg, ord)    __afetch_add(obj, arg)

static inline int __aload(__atomic_int *obj)
{
	return *obj;
}

static inline int __astore(__atomic_int *obj, int arg)
{
	*obj = arg;

	return *obj;
}

static inline int __aexchange(__atomic_int *obj, int arg)
{
	int tmp = *obj;

	*obj = arg;

	return tmp;
}

static inline int __afetch_add(__atomic_int *obj, int arg)
{
	*obj += arg;

	return *obj;
}

#endif /* !CONFIG_ATOMIC */

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ATOMIC_H_ */
