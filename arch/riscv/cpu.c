/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>

#include <bmetal/arch/cpu.h>
#include <bmetal/printk.h>

int __arch_riscv_get_cpu_id(void)
{
	int tmp;

	__asm volatile ("csrr %0, mhartid" : "=r"(tmp));

	return tmp;
}

void __arch_riscv_wait_interrupt(void)
{
	__asm volatile ("wfi");
}

int __arch_riscv_get_arg(__arch_riscv_user_regs_t *regs, enum __arch_arg_type t, uintptr_t *val)
{
	uintptr_t v;

	switch (t) {
	case __ARCH_ARG_TYPE_1:
	case __ARCH_ARG_TYPE_RETVAL:
		v = regs->a0;
		break;
	case __ARCH_ARG_TYPE_2:
		v = regs->a1;
		break;
	case __ARCH_ARG_TYPE_3:
		v = regs->a2;
		break;
	case __ARCH_ARG_TYPE_4:
		v = regs->a3;
		break;
	case __ARCH_ARG_TYPE_5:
		v = regs->a4;
		break;
	case __ARCH_ARG_TYPE_6:
		v = regs->a5;
		break;
	case __ARCH_ARG_TYPE_RETADDR:
		v = regs->ra;
		break;
	case __ARCH_ARG_TYPE_STACK:
		v = regs->sp;
		break;
	case __ARCH_ARG_TYPE_INTADDR:
		v = regs->mepc;
		break;
	default:
		printk("riscv_get_arg: unknown arg type %d\n", t);
		return -EINVAL;
	}

	if (val) {
		*val = v;
	}

	return 0;
}

int __arch_riscv_set_arg(__arch_riscv_user_regs_t *regs, enum __arch_arg_type t, uintptr_t val)
{
	uintptr_t *v;

	switch (t) {
	case __ARCH_ARG_TYPE_1:
	case __ARCH_ARG_TYPE_RETVAL:
		v = &regs->a0;
		break;
	case __ARCH_ARG_TYPE_2:
		v = &regs->a1;
		break;
	case __ARCH_ARG_TYPE_3:
		v = &regs->a2;
		break;
	case __ARCH_ARG_TYPE_4:
		v = &regs->a3;
		break;
	case __ARCH_ARG_TYPE_5:
		v = &regs->a4;
		break;
	case __ARCH_ARG_TYPE_6:
		v = &regs->a5;
		break;
	case __ARCH_ARG_TYPE_RETADDR:
		v = &regs->ra;
		break;
	case __ARCH_ARG_TYPE_STACK:
		v = &regs->sp;
		break;
	case __ARCH_ARG_TYPE_INTADDR:
		v = &regs->mepc;
		break;
	default:
		printk("riscv_set_arg: unknown arg type %d\n", t);
		return -EINVAL;
	}

	*v = val;

	return 0;
}
