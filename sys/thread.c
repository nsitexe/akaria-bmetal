/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdatomic.h>

#include <bmetal/thread.h>
#include <bmetal/arch.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/sys/string.h>

static struct __proc_info __pi;
/* Each CPU has 2 threads (idle and task) */
static struct __thread_info __ti[CONFIG_NUM_CORES * 2];
static atomic_int uniq_tid = 1;

static int alloc_tid(void)
{
	return atomic_fetch_add(&uniq_tid, 1);
}

struct __proc_info *__proc_create(void)
{
	if (__pi.avail) {
		printk("proc_create: already created.\n");
		return NULL;
	}

	__pi.avail = 1;

	return &__pi;
}

struct __proc_info *__proc_get_current(void)
{
	if (!__pi.avail) {
		printk("proc_get_current: no process.\n");
		return NULL;
	}

	return &__pi;
}

pid_t __proc_get_pid(void)
{
	struct __proc_info *pi = __proc_get_current();

	return pi->pid;
}

int __thread_get_leader(struct __thread_info *ti)
{
	return ti->leader;
}

void __thread_set_leader(struct __thread_info *ti, int l)
{
	ti->leader = l;
}

void __thread_idle_main(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti = NULL;
	int r;

	r = __cpu_on_wakeup();
	if (r) {
		printk("idle: failed to callback on_wakeup.\n");
	}

	__intr_enable_local();

	while (__cpu_get_running(cpu)) {
		ti = __cpu_get_thread_task(cpu);
		while (!ti) {
			/* Wait for notification from other cores */
			__cpu_wait_interrupt();

			drmb();

			ti = __cpu_get_thread_task(cpu);
		}

		/* Switch to task thread from idle thread */
		__arch_context_switch();
	}

	__intr_disable_local();

	r = __cpu_on_sleep();
	if (r) {
		printk("idle: failed to callback on_sleep.\n");
	}

	/* tentative: currently unreachable */
	while (1) {
	}
}

struct __thread_info *__thread_create(struct __proc_info *pi)
{
	struct __thread_info *ti = NULL;
	int found = 0, r;

	__spinlock_lock(&pi->lock);

	for (int i = 0; i < ARRAY_OF(__ti); i++) {
		ti = __thread_get_raw(i);

		if (!ti->avail) {
			found = 1;
			break;
		}
	}
	if (!found) {
		printk("create_thread: reach to limit.\n");

		__spinlock_unlock(&pi->lock);
		return NULL;
	}

	ti->pi = pi;
	ti->tid = alloc_tid();
	ti->avail = 1;
	ti->running = 0;

	ti->sp = 0;
	ti->ctid = 0;
	ti->ptid = 0;

	r = __arch_thread_init(ti);
	if (r) {
		printk("create_thread: failed to arch_thread_init.\n");

		__spinlock_unlock(&pi->lock);
		return NULL;
	}

	__spinlock_unlock(&pi->lock);

	return ti;
}

int __thread_destroy(struct __thread_info *ti)
{
	if (!ti) {
		return -EINVAL;
	}
	if (ti->running) {
		printk("destroy_thread: thread %d is running.\n", ti->tid);
		return -EBUSY;
	}

	ti->avail = 0;

	return 0;
}

int __thread_run(struct __thread_info *ti, struct __cpu_device *cpu)
{
	ti->running = 1;
	ti->cpu = cpu;
	__cpu_set_thread_task(cpu, ti);

	return 0;
}

int __thread_stop(struct __thread_info *ti)
{
	__cpu_set_thread_task(ti->cpu, NULL);
	ti->cpu = NULL;
	ti->running = 0;

	return 0;
}

struct __thread_info *__thread_get_raw(int n)
{
	if (n < 0 || ARRAY_OF(__ti) <= n) {
		return NULL;
	}

	return &__ti[n];
}

struct __thread_info *__thread_get(pid_t tid)
{
	for (int i = 0; i < ARRAY_OF(__ti); i++) {
		struct __thread_info *ti = __thread_get_raw(i);

		if (ti->tid == tid) {
			return ti;
		}
	}

	return NULL;
}

struct __thread_info *__thread_get_current(void)
{
	return __cpu_get_thread(__cpu_get_current());
}

pid_t __thread_get_tid(void)
{
	struct __thread_info *ti = __thread_get_current();

	return ti->tid;
}

int __thread_context_switch(void)
{
	int r;

	__smp_lock();
	r = __thread_context_switch_nolock();
	__smp_unlock();

	return r;
}

int __thread_context_switch_nolock(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti, *ti_idle, *ti_task;

	drmb();

	ti = __cpu_get_thread(cpu);
	ti_idle = __cpu_get_thread_idle(cpu);
	ti_task = __cpu_get_thread_task(cpu);

	/*
	 * Save the thread context.
	 *
	 * idle to task: save context of the idle thread.
	 * task to idle: currently we do not need to save context because the
	 *   task thread will be destroy soon and never switch to task again.
	 */
	if (ti == ti_idle) {
		kmemcpy(&ti->regs, cpu->regs, sizeof(__arch_user_regs_t));
	}

	if (ti && ti_task) {
		/* Switch to task */
		kmemcpy(cpu->regs, &ti_task->regs, sizeof(__arch_user_regs_t));
		__cpu_set_thread(cpu, ti_task);
	} else {
		/* Switch to idle */
		kmemcpy(cpu->regs, &ti_idle->regs, sizeof(__arch_user_regs_t));
		__cpu_set_thread(cpu, ti_idle);
	}

	dwmb();

	return 0;
}
