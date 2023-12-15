/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SMP_H_
#define BAREMETAL_CRT_SMP_H_

#include <bmetal/bmetal.h>

#include <bmetal/thread.h>
#include <bmetal/driver/cpu.h>

#if !defined(__ASSEMBLER__)

int k_smp_lock(void);
int k_smp_unlock(void);
int k_smp_find_idle_cpu(struct k_cpu_device **cpu_idle);
int k_smp_find_idle_thread(struct k_thread_info **ti_idle);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SMP_H_ */
