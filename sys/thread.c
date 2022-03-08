/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdatomic.h>

#include <bmetal/thread.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/cpu.h>

static struct __proc_info __pi;
static struct __thread_info __ti[CONFIG_NUM_CORES];
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

struct __thread_info *__thread_create(struct __proc_info *pi)
{
	struct __thread_info *ti = NULL;

	__spinlock_lock(&pi->lock);

	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		ti = __thread_get_raw(i);

		if (!ti->avail) {
			ti->pi = pi;
			ti->tid = alloc_tid();
			ti->avail = 1;

			break;
		}
	}
	if (ti == NULL) {
		printk("create_thread: reach to limit.\n");
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
	cpu->ti = ti;

	return 0;
}

int __thread_stop(struct __thread_info *ti)
{
	ti->cpu->ti = NULL;
	ti->cpu = NULL;
	ti->running = 0;

	return 0;
}

struct __thread_info *__thread_get_raw(int n)
{
	if (n < 0 || CONFIG_NUM_CORES <= n) {
		return NULL;
	}

	return &__ti[n];
}

struct __thread_info *__thread_get(pid_t tid)
{
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
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
