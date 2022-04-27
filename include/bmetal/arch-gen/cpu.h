/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_CPU_H_
#define BAREMETAL_CRT_ARCH_GEN_CPU_H_

#include <bmetal/bmetal.h>

enum __arch_arg_type {
	__ARCH_ARG_TYPE_1,
	__ARCH_ARG_TYPE_2,
	__ARCH_ARG_TYPE_3,
	__ARCH_ARG_TYPE_4,
	__ARCH_ARG_TYPE_5,
	__ARCH_ARG_TYPE_6,
	__ARCH_ARG_TYPE_INTADDR,
	__ARCH_ARG_TYPE_RETADDR,
	__ARCH_ARG_TYPE_RETVAL,
	__ARCH_ARG_TYPE_STACK,
};

#ifndef __arch_user_regs_t
#  error Please implement arch_user_regs_t type.
#endif

#ifndef __arch_get_cpu_id
#  error Please implement arch_get_cpu_id() function.
#endif

#ifndef __arch_get_arg
#  error Please implement arch_get_arg() function.
#endif

#ifndef __arch_set_arg
#  error Please implement arch_set_arg() function.
#endif

#ifndef __arch_copy_current_context
#  error Please implement arch_copy_current_context() function.
#endif

#ifndef __arch_context_switch
#  error Please implement arch_context_switch() function.
#endif

#ifndef __arch_get_cpu_id
#  error Please implement arch_get_cpu_id() function.
#endif

#endif /* BAREMETAL_CRT_ARCH_GEN_CPU_H_ */
