/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>

#include <bmetal/arch/intr.h>
#include <bmetal/printk.h>
#include <bmetal/arch/cpu.h>

static struct __event_handler *int_handlers[RV_CAUSE_INT_MAX_NUM];
static struct __event_handler *exc_handlers[RV_CAUSE_EXC_MAX_NUM];

int __arch_riscv_interrupt(uintptr_t mcause)
{
	struct __event_handler *h;
	int cause = mcause & 0x7fffffff;
	int r;

	if (cause >= RV_CAUSE_INT_MAX_NUM) {
		printk("arch_riscv_interrupt: illegal interrupt number %d.\n", cause);
		return 0;
	}

	h = int_handlers[cause];
	if (h && h->func) {
		r = h->func(cause, h);
		if (r == EVENT_HANDLED) {
			__asm volatile("csrc mip, %0" : : "r"(1 << cause));
		}
	}

	return 0;
}

int __arch_riscv_exception(uintptr_t mcause)
{
	struct __event_handler *h;
	int cause = mcause & 0x7fffffff;
	int r;

	if (cause >= RV_CAUSE_EXC_MAX_NUM) {
		printk("arch_riscv_exception: illegal exception number %d.\n", cause);
		return 0;
	}

	h = exc_handlers[cause];
	if (h && h->func) {
		r = h->func(cause, h);
		if (r == EVENT_HANDLED) {
			/* Do nothing */
		}
	}

	/* FIXME: for debug */
	while (1) {
		__asm volatile("wfi");
	}

	return 0;
}

int __arch_riscv_get_intr_handler(int event, struct __event_handler **hnd)
{
	struct __event_handler *h;

	switch (event) {
	case RV_CAUSE_INT_U_SW:
	case RV_CAUSE_INT_S_SW:
	case RV_CAUSE_INT_M_SW:
	case RV_CAUSE_INT_U_TM:
	case RV_CAUSE_INT_S_TM:
	case RV_CAUSE_INT_M_TM:
	case RV_CAUSE_INT_U_EX:
	case RV_CAUSE_INT_S_EX:
	case RV_CAUSE_INT_M_EX:
		h = int_handlers[event];
		break;
	default:
		printk("get_intr_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	if (hnd) {
		*hnd = h;
	}

	return 0;
}

int __arch_riscv_set_intr_handler(int event, struct __event_handler *hnd)
{
	switch (event) {
	case RV_CAUSE_INT_U_SW:
	case RV_CAUSE_INT_S_SW:
	case RV_CAUSE_INT_M_SW:
	case RV_CAUSE_INT_U_TM:
	case RV_CAUSE_INT_S_TM:
	case RV_CAUSE_INT_M_TM:
	case RV_CAUSE_INT_U_EX:
	case RV_CAUSE_INT_S_EX:
	case RV_CAUSE_INT_M_EX:
		int_handlers[event] = hnd;
		break;
	default:
		printk("set_intr_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	return 0;
}

int __arch_riscv_get_exc_handler(int event, struct __event_handler **hnd)
{
	struct __event_handler *h;

	switch (event) {
	case RV_CAUSE_EXC_INS_ADDR:
	case RV_CAUSE_EXC_INS_ACC:
	case RV_CAUSE_EXC_ILL:
	case RV_CAUSE_EXC_BRK:
	case RV_CAUSE_EXC_LD_ADDR:
	case RV_CAUSE_EXC_LD_FAULT:
	case RV_CAUSE_EXC_ST_ADDR:
	case RV_CAUSE_EXC_ST_FAULT:
	case RV_CAUSE_EXC_ECALL_U:
	case RV_CAUSE_EXC_ECALL_S:
	case RV_CAUSE_EXC_ECALL_M:
	case RV_CAUSE_EXC_INS_PAGE_FAULT:
	case RV_CAUSE_EXC_LD_PAGE_FAULT:
	case RV_CAUSE_EXC_ST_PAGE_FAULT:
		h = exc_handlers[event];
		break;
	default:
		printk("get_exc_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	if (hnd) {
		*hnd = h;
	}

	return 0;
}

int __arch_riscv_set_exc_handler(int event, struct __event_handler *hnd)
{
	switch (event) {
	case RV_CAUSE_EXC_INS_ADDR:
	case RV_CAUSE_EXC_INS_ACC:
	case RV_CAUSE_EXC_ILL:
	case RV_CAUSE_EXC_BRK:
	case RV_CAUSE_EXC_LD_ADDR:
	case RV_CAUSE_EXC_LD_FAULT:
	case RV_CAUSE_EXC_ST_ADDR:
	case RV_CAUSE_EXC_ST_FAULT:
	case RV_CAUSE_EXC_ECALL_U:
	case RV_CAUSE_EXC_ECALL_S:
	case RV_CAUSE_EXC_ECALL_M:
	case RV_CAUSE_EXC_INS_PAGE_FAULT:
	case RV_CAUSE_EXC_LD_PAGE_FAULT:
	case RV_CAUSE_EXC_ST_PAGE_FAULT:
		exc_handlers[event] = hnd;
		break;
	default:
		printk("set_exc_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	return 0;
}

void __arch_riscv_intr_enable_local(void)
{
	long val = XSTATUS_IE;

	__asm volatile ("csrs mstatus, %0" : : "r"(val));
}

void __arch_riscv_intr_disable_local(void)
{
	long val = XSTATUS_IE;

	__asm volatile ("csrc mstatus, %0" : : "r"(val));
}

void __arch_riscv_intr_restore_local(const long status)
{
	__asm volatile ("csrw mstatus, %0" : : "r"(status));
}

void __arch_riscv_intr_save_local(long *status)
{
	long tmp, val = XSTATUS_IE;

	__asm volatile ("csrrc %0, mstatus, %1" : "=r"(tmp) : "r"(val));
	*status = tmp;
}
