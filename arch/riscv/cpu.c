/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/cpu.h>

int __arch_riscv_get_cpu_id(void)
{
	int tmp;

	__asm volatile ("csrr %0, mhartid" : "=r"(tmp));

	return tmp;
}
