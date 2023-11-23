/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SMP_H_
#define BAREMETAL_CRT_SMP_H_

#include <bmetal/bmetal.h>

#include <bmetal/thread.h>
#include <bmetal/drivers/cpu.h>

#if !defined(__ASSEMBLER__)

int __smp_lock(void);
int __smp_unlock(void);
int __smp_find_idle_cpu(struct k_cpu_device **cpu_idle);
int __smp_find_idle_thread(struct __thread_info **ti_idle);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SMP_H_ */
