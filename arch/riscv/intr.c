/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/intr.h>
#include <bmetal/printk.h>
#include <bmetal/arch/cpu.h>
#include <bmetal/sys/errno.h>

static struct k_event_handler *k_int_handlers[RV_CAUSE_INT_MAX_NUM];
static struct k_event_handler *k_exc_handlers[RV_CAUSE_EXC_MAX_NUM];

int k_arch_riscv_interrupt(uintptr_t mcause)
{
	struct k_event_handler *h;
	int cause = mcause & 0x7fffffff;
	int r;

	if (cause >= RV_CAUSE_INT_MAX_NUM) {
		k_pri_warn("arch_riscv_interrupt: illegal interrupt number %d.\n", cause);
		return 0;
	}

	h = k_int_handlers[cause];
	if (h && h->func) {
		r = h->func(cause, h);
		if (r == EVENT_HANDLED) {
			__asm volatile ("csrc mip, %0" : : "r"(1 << cause));
		}
	}

	return 0;
}

int k_arch_riscv_exception(uintptr_t mcause)
{
	struct k_event_handler *h;
	int cause = mcause & 0x7fffffff;
	int r;

	if (cause >= RV_CAUSE_EXC_MAX_NUM) {
		k_pri_warn("arch_riscv_exception: illegal exception number %d.\n", cause);
		return 0;
	}

	h = k_exc_handlers[cause];
	if (h && h->func) {
		r = h->func(cause, h);
		if (r == EVENT_HANDLED) {
			/* Do nothing */
		}
	}

	/* FIXME: for debug */
	while (1) {
		__asm volatile ("wfi");
	}

	return 0;
}

int k_arch_riscv_get_intr_handler(int event, struct k_event_handler **hnd)
{
	struct k_event_handler *h;

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
		h = k_int_handlers[event];
		break;
	default:
		k_pri_warn("get_intr_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	if (hnd) {
		*hnd = h;
	}

	return 0;
}

int k_arch_riscv_set_intr_handler(int event, struct k_event_handler *hnd)
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
		k_int_handlers[event] = hnd;
		break;
	default:
		k_pri_warn("set_intr_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	return 0;
}

int k_arch_riscv_get_exc_handler(int event, struct k_event_handler **hnd)
{
	struct k_event_handler *h;

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
		h = k_exc_handlers[event];
		break;
	default:
		k_pri_warn("get_exc_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	if (hnd) {
		*hnd = h;
	}

	return 0;
}

int k_arch_riscv_set_exc_handler(int event, struct k_event_handler *hnd)
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
		k_exc_handlers[event] = hnd;
		break;
	default:
		k_pri_warn("set_exc_handler: unknown event number %d.\n", event);
		return -EINVAL;
	}

	return 0;
}

void k_arch_riscv_intr_enable_local(void)
{
	long val = XSTATUS_IE;

	__asm volatile ("csrs mstatus, %0" : : "r"(val));
}

void k_arch_riscv_intr_disable_local(void)
{
	long val = XSTATUS_IE;

	__asm volatile ("csrc mstatus, %0" : : "r"(val));
}

void k_arch_riscv_intr_restore_local(const long status)
{
	__asm volatile ("csrw mstatus, %0" : : "r"(status));
}

void k_arch_riscv_intr_save_local(long *status)
{
	long tmp, val = XSTATUS_IE;

	__asm volatile ("csrrc %0, mstatus, %1" : "=r"(tmp) : "r"(val));
	*status = tmp;
}
