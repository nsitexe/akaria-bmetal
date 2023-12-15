/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_CPU_H_
#define BAREMETAL_CRT_ARCH_GEN_CPU_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

enum k_arch_arg_type {
	K_ARCH_ARG_TYPE_1,
	K_ARCH_ARG_TYPE_2,
	K_ARCH_ARG_TYPE_3,
	K_ARCH_ARG_TYPE_4,
	K_ARCH_ARG_TYPE_5,
	K_ARCH_ARG_TYPE_6,
	K_ARCH_ARG_TYPE_INTADDR,
	K_ARCH_ARG_TYPE_RETADDR,
	K_ARCH_ARG_TYPE_RETVAL,
	K_ARCH_ARG_TYPE_STACK,
	K_ARCH_ARG_TYPE_STACK_INTR,
	K_ARCH_ARG_TYPE_TLS,
};

#ifndef k_arch_user_regs_t
#  error Please implement k_arch_user_regs_t type.
#endif

#ifndef k_arch_get_cpu_id
#  error Please implement k_arch_get_cpu_id() function.
#endif

#ifndef k_arch_wait_interrupt
#  error Please implement k_arch_wait_interrupt() function.
#endif

#ifndef k_arch_get_arg
#  error Please implement k_arch_get_arg() function.
#endif

#ifndef k_arch_set_arg
#  error Please implement k_arch_set_arg() function.
#endif

#ifndef k_arch_copy_current_context
#  error Please implement k_arch_copy_current_context() function.
#endif

#ifndef k_arch_context_switch
#  error Please implement k_arch_context_switch() function.
#endif

#ifndef k_arch_get_cpu_id
#  error Please implement k_arch_get_cpu_id() function.
#endif

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_GEN_CPU_H_ */
