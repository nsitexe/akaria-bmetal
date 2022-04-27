/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_CPU_H_
#define BAREMETAL_CRT_ARCH_GEN_CPU_H_

#include <bmetal/bmetal.h>

#ifndef __arch_user_regs_t
#  error Please implement arch_user_regs_t type.
#endif

#ifndef __arch_get_cpu_id
#  error Please implement arch_get_cpu_id() function.
#endif

#endif /* BAREMETAL_CRT_ARCH_GEN_CPU_H_ */
