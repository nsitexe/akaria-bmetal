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

typedef atomic_int k_atomic_int;
typedef atomic_long k_atomic_long;

#define k_aload_add_explicit(obj, ord)          atomic_load_add_explicit(obj, ord)
#define k_aload_add(obj)                        atomic_load_add(obj)
#define k_astore_explicit(obj, arg, ord)        atomic_store_explicit(obj, arg, ord)
#define k_astore(obj, arg)                      atomic_store(obj, arg)
#define k_aexchange_explicit(obj, arg, ord)     atomic_exchange_explicit(obj, arg, ord)
#define k_aexchange(obj, arg)                   atomic_exchange(obj, arg)
#define k_afetch_add_explicit(obj, arg, ord)    atomic_fetch_add_explicit(obj, arg, ord)
#define k_afetch_add(obj, arg)                  atomic_fetch_add(obj, arg)

#else /* CONFIG_ATOMIC */

typedef volatile int k_atomic_int;
typedef volatile long k_atomic_long;

#define k_aload_explicit(obj, arg, ord)         k_aload(obj)
#define k_astore_explicit(obj, arg, ord)        k_astore(obj, arg)
#define k_aexchange_explicit(obj, arg, ord)     k_aexchange(obj, arg)
#define k_afetch_add_explicit(obj, arg, ord)    k_afetch_add(obj, arg)

static inline int k_aload(k_atomic_int *obj)
{
	return *obj;
}

static inline int k_astore(k_atomic_int *obj, int arg)
{
	*obj = arg;

	return *obj;
}

static inline int k_aexchange(k_atomic_int *obj, int arg)
{
	int tmp = *obj;

	*obj = arg;

	return tmp;
}

static inline int k_afetch_add(k_atomic_int *obj, int arg)
{
	*obj += arg;

	return *obj;
}

#endif /* !CONFIG_ATOMIC */

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ATOMIC_H_ */
