/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/thread.h>
#include <bmetal/arch.h>
#include <bmetal/atomic.h>
#include <bmetal/init.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/sys/errno.h>
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
		pri_warn("proc_create: already created.\n");
		return NULL;
	}

	k_pi.avail = 1;

	return &k_pi;
}

struct k_proc_info *k_proc_get_current(void)
{
	if (!k_pi.avail) {
		pri_err("proc_get_current: no process.\n");
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
		pri_warn("idle: failed to callback on_wakeup.\n");
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
		pri_warn("idle: failed to callback on_sleep.\n");
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
		pri_warn("create_thread: reach to limit.\n");

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
		pri_warn("create_thread: failed to arch_thread_init.\n");

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
		pri_warn("destroy_thread: thread %d is running.\n", ti->tid);
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
