/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/arch/cpu.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>

int k_arch_riscv_get_cpu_id(void)
{
	int tmp;

	__asm volatile ("csrr %0, mhartid" : "=r"(tmp));

	return tmp;
}

void k_arch_riscv_wait_interrupt(void)
{
	__asm volatile ("wfi");
}

int k_arch_riscv_get_arg(k_arch_riscv_user_regs_t *regs, enum k_arch_arg_type t, uintptr_t *val)
{
	uintptr_t v;

	switch (t) {
	case K_ARCH_ARG_TYPE_1:
	case K_ARCH_ARG_TYPE_RETVAL:
		v = regs->a0;
		break;
	case K_ARCH_ARG_TYPE_2:
		v = regs->a1;
		break;
	case K_ARCH_ARG_TYPE_3:
		v = regs->a2;
		break;
	case K_ARCH_ARG_TYPE_4:
		v = regs->a3;
		break;
	case K_ARCH_ARG_TYPE_5:
		v = regs->a4;
		break;
	case K_ARCH_ARG_TYPE_6:
		v = regs->a5;
		break;
	case K_ARCH_ARG_TYPE_RETADDR:
		v = regs->ra;
		break;
	case K_ARCH_ARG_TYPE_STACK:
		v = regs->sp;
		break;
	case K_ARCH_ARG_TYPE_STACK_INTR:
		v = regs->mscratch;
		break;
	case K_ARCH_ARG_TYPE_TLS:
		v = regs->tp;
		break;
	case K_ARCH_ARG_TYPE_INTADDR:
		v = regs->mepc;
		break;
	default:
		pri_warn("riscv_get_arg: unknown arg type %d\n", t);
		return -EINVAL;
	}

	if (val) {
		*val = v;
	}

	return 0;
}

int k_arch_riscv_set_arg(k_arch_riscv_user_regs_t *regs, enum k_arch_arg_type t, uintptr_t val)
{
	uintptr_t *v;

	switch (t) {
	case K_ARCH_ARG_TYPE_1:
	case K_ARCH_ARG_TYPE_RETVAL:
		v = &regs->a0;
		break;
	case K_ARCH_ARG_TYPE_2:
		v = &regs->a1;
		break;
	case K_ARCH_ARG_TYPE_3:
		v = &regs->a2;
		break;
	case K_ARCH_ARG_TYPE_4:
		v = &regs->a3;
		break;
	case K_ARCH_ARG_TYPE_5:
		v = &regs->a4;
		break;
	case K_ARCH_ARG_TYPE_6:
		v = &regs->a5;
		break;
	case K_ARCH_ARG_TYPE_RETADDR:
		v = &regs->ra;
		break;
	case K_ARCH_ARG_TYPE_STACK:
		v = &regs->sp;
		break;
	case K_ARCH_ARG_TYPE_STACK_INTR:
		v = &regs->mscratch;
		break;
	case K_ARCH_ARG_TYPE_TLS:
		v = &regs->tp;
		break;
	case K_ARCH_ARG_TYPE_INTADDR:
		v = &regs->mepc;
		break;
	default:
		pri_warn("riscv_set_arg: unknown arg type %d\n", t);
		return -EINVAL;
	}

	*v = val;

	return 0;
}
