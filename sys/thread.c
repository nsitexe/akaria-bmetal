/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/thread.h>
#include <bmetal/arch.h>
#include <bmetal/atomic.h>
#include <bmetal/init.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/driver/cpu.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/sched.h>
#include <bmetal/sys/string.h>

static struct k_proc_info k_pi;
/* Each CPU has 2 threads (idle and task) */
static struct k_thread_info k_ti[CONFIG_NUM_CORES * 2];
static k_atomic_int uniq_tid = 1;

static int alloc_tid(void)
{
	return k_afetch_add(&uniq_tid, 1);
}

struct k_proc_info *k_proc_create(void)
{
	if (k_pi.avail) {
		k_pri_warn("proc_create: already created.\n");
		return NULL;
	}

	k_pi.avail = 1;

	return &k_pi;
}

struct k_proc_info *k_proc_get_current(void)
{
	if (!k_pi.avail) {
		k_pri_err("proc_get_current: no process.\n");
		return NULL;
	}

	return &k_pi;
}

pid_t k_proc_get_pid(void)
{
	struct k_proc_info *pi = k_proc_get_current();

	return pi->pid;
}

struct k_thread_info *k_proc_get_leader(struct k_proc_info *pi)
{
	return pi->leader;
}

int k_proc_set_leader(struct k_proc_info *pi, struct k_thread_info *ti)
{
	pi->leader = ti;

	return 0;
}

void k_thread_idle_main(int leader)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	struct k_thread_info *ti = NULL;
	int r;

	k_intr_disable_local();

	r = k_cpu_on_wakeup();
	if (r) {
		k_pri_warn("idle: failed to callback on_wakeup.\n");
	}

	k_intr_enable_local();

	while (k_cpu_get_running(cpu)) {
		ti = k_cpu_get_thread_task(cpu);
		while (k_cpu_get_running(cpu) && !ti) {
			/* Wait for notification from other cores */
			k_cpu_wait_interrupt();

			drmb();

			ti = k_cpu_get_thread_task(cpu);
		}

		/* Switch to task thread from idle thread */
		k_arch_context_switch();
	}

	k_intr_disable_local();

	r = k_cpu_on_sleep();
	if (r) {
		k_pri_warn("idle: failed to callback on_sleep.\n");
	}

	if (leader) {
		k_fini_system();
	}

	while (1) {
		k_cpu_wait_interrupt();
	}
}

struct k_thread_info *k_thread_create(struct k_proc_info *pi)
{
	struct k_thread_info *ti = NULL;
	int found = 0, r;

	k_spinlock_lock(&pi->lock);

	for (int i = 0; i < ARRAY_OF(k_ti); i++) {
		ti = k_thread_get_raw(i);

		if (!ti->avail) {
			found = 1;
			break;
		}
	}
	if (!found) {
		k_pri_warn("create_thread: reach to limit.\n");

		k_spinlock_unlock(&pi->lock);
		return NULL;
	}

	ti->pi = pi;
	ti->tid = alloc_tid();
	ti->avail = 1;
	ti->running = 0;

	ti->sp = 0;
	ti->ctid = 0;
	ti->ptid = 0;

	r = k_arch_thread_init(ti);
	if (r) {
		k_pri_warn("create_thread: failed to arch_thread_init.\n");

		k_spinlock_unlock(&pi->lock);
		return NULL;
	}

	k_spinlock_unlock(&pi->lock);

	return ti;
}

int k_thread_destroy(struct k_thread_info *ti)
{
	if (!ti) {
		return -EINVAL;
	}
	if (ti->running) {
		k_pri_warn("destroy_thread: thread %d is running.\n", ti->tid);
		return -EBUSY;
	}

	ti->avail = 0;

	return 0;
}

int k_thread_run(struct k_thread_info *ti, struct k_cpu_device *cpu)
{
	ti->running = 1;
	ti->cpu = cpu;
	k_cpu_set_thread_task(cpu, ti);

	return 0;
}

int k_thread_stop(struct k_thread_info *ti)
{
	k_cpu_set_thread_task(ti->cpu, NULL);
	ti->cpu = NULL;
	ti->running = 0;

	return 0;
}

struct k_thread_info *k_thread_get_raw(int n)
{
	if (n < 0 || ARRAY_OF(k_ti) <= n) {
		return NULL;
	}

	return &k_ti[n];
}

struct k_thread_info *k_thread_get(pid_t tid)
{
	for (int i = 0; i < ARRAY_OF(k_ti); i++) {
		struct k_thread_info *ti = k_thread_get_raw(i);

		if (ti->tid == tid) {
			return ti;
		}
	}

	return NULL;
}

struct k_thread_info *k_thread_get_current(void)
{
	return k_cpu_get_thread(k_cpu_get_current());
}

pid_t k_thread_get_tid(void)
{
	struct k_thread_info *ti = k_thread_get_current();

	return ti->tid;
}

pid_t k_thread_clone(const struct k_clone_args *args)
{
/*unsigned long flags, void *child_stack, void *ptid, void *tls, void *ctid*/
	struct k_cpu_device *cpu_cur = k_cpu_get_current(), *cpu;
	struct k_proc_info *pi = k_proc_get_current();
	struct k_thread_info *ti;
	size_t pos_intr;
	int need_ctid = 0, need_ptid = 0, need_tls = 0;
	int r;

	if (args->flags & (CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID)) {
		if (!args->ctid) {
			k_pri_warn("sys_clone: Need ctid but ctid is NULL.\n");
			return -EFAULT;
		}
		need_ctid = 1;
	}
	if (args->flags & CLONE_PARENT_SETTID) {
		if (!args->ptid) {
			k_pri_warn("sys_clone: Need ptid but ptid is NULL.\n");
			return -EFAULT;
		}
		need_ptid = 1;
	}
	if (args->flags & CLONE_SETTLS) {
		if (!args->tls) {
			k_pri_warn("sys_clone: Need tls but tls is NULL.\n");
			return -EFAULT;
		}
		need_tls = 1;
	}

	k_smp_lock();

	r = k_smp_find_idle_cpu(&cpu);
	if (r) {
		k_smp_unlock();
		return r;
	}

	k_smp_unlock();

	k_cpu_lock(cpu);

	/* Init thread info */
	ti = k_thread_create(pi);
	if (!ti) {
		r = -ENOMEM;
		goto err_out;
	}

	ti->flags = args->flags;
	ti->ctid = NULL;
	if (need_ctid) {
		ti->ctid = args->ctid;
	}
	ti->ptid = NULL;
	if (need_ptid) {
		ti->ptid = args->ptid;
	}
	ti->tls = NULL;
	if (need_tls) {
		ti->tls = args->tls;
	}

	/* Notify to user space */
	if (args->flags & CLONE_CHILD_SETTID) {
		*ti->ctid = ti->tid;
	}
	if (args->flags & CLONE_PARENT_SETTID) {
		*ti->ptid = ti->tid;
	}

	/* Copy user regs to the initial stack of new thread */
	ti->sp = args->child_stack;
	k_memcpy(&ti->regs, cpu_cur->regs, sizeof(k_arch_user_regs_t));

	pos_intr = (cpu->id_cpu + 1) * CONFIG_INTR_STACK_SIZE;

	/* Return value and stack for new thread */
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_RETVAL, 0);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_STACK, (uintptr_t)ti->sp);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_STACK_INTR, (uintptr_t)&k_stack_intr[pos_intr]);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_TLS, (uintptr_t)ti->tls);

	r = k_thread_run(ti, cpu);
	if (r) {
		goto err_out2;
	}

	r = k_cpu_raise_ipi(ti->cpu, NULL);
	if (r) {
		goto err_out3;
	}

	dwmb();
	k_cpu_unlock(cpu);

	/* Return value for current thread */
	return ti->tid;

err_out3:
	k_thread_stop(ti);

err_out2:
	k_thread_destroy(ti);

err_out:
	k_cpu_unlock(cpu);

	return r;
}

int k_thread_context_switch(void)
{
	int r;

	k_smp_lock();
	r = k_thread_context_switch_nolock();
	k_smp_unlock();

	return r;
}

int k_thread_context_switch_nolock(void)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	struct k_thread_info *ti, *ti_idle, *ti_task;

	ti = k_cpu_get_thread(cpu);
	ti_idle = k_cpu_get_thread_idle(cpu);
	ti_task = k_cpu_get_thread_task(cpu);

	/*
	 * Save the thread context.
	 *
	 * idle to task: save context of the idle thread.
	 * task to idle: currently we do not need to save context because the
	 *   task thread will be destroy soon and never switch to task again.
	 */
	if (ti == ti_idle) {
		k_memcpy(&ti->regs, cpu->regs, sizeof(k_arch_user_regs_t));
	}

	if (ti && ti_task) {
		/* Switch to task */
		k_memcpy(cpu->regs, &ti_task->regs, sizeof(k_arch_user_regs_t));
		k_cpu_set_thread(cpu, ti_task);
	} else {
		/* Switch to idle */
		k_memcpy(cpu->regs, &ti_idle->regs, sizeof(k_arch_user_regs_t));
		k_cpu_set_thread(cpu, ti_idle);
	}

	return 0;
}
