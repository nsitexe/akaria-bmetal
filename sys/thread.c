/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdatomic.h>

#include <bmetal/thread.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/cpu.h>

static struct __process_info __pi;
static struct __thread_info __ti[CONFIG_NUM_CORES];
static atomic_int uniq_tid = 1;

static int alloc_tid(void)
{
	return atomic_fetch_add(&uniq_tid, 1);
}

struct __process_info *__create_process(void)
{
	if (__pi.avail) {
		printk("create_process: already created.\n");
		return NULL;
	}

	__pi.avail = 1;

	return &__pi;
}

struct __process_info *__get_current_process(void)
{
	if (!__pi.avail) {
		printk("get_process: no process.\n");
		return NULL;
	}

	return &__pi;
}

pid_t __get_pid(void)
{
	struct __process_info *pi = __get_current_process();

	return pi->pid;
}

struct __thread_info *__create_thread(struct __process_info *pi)
{
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		struct __thread_info *ti = __get_raw_thread(i);

		if (!ti->avail) {
			ti->pi = pi;
			ti->tid = alloc_tid();
			ti->avail = 1;

			return ti;
		}
	}
	printk("create_thread: reach to limit.\n");

	return NULL;
}

int __destroy_thread(struct __thread_info *ti)
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

int __run_thread(struct __thread_info *ti, struct __cpu_device *cpu)
{
	ti->running = 1;
	ti->cpu = cpu;
	cpu->ti = ti;

	return 0;
}

int __stop_thread(struct __thread_info *ti)
{
	ti->cpu->ti = NULL;
	ti->cpu = NULL;
	ti->running = 0;

	return 0;
}

struct __thread_info *__get_raw_thread(int n)
{
	if (n < 0 || CONFIG_NUM_CORES <= n) {
		return NULL;
	}

	return &__ti[n];
}

struct __thread_info *__get_thread(pid_t tid)
{
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		struct __thread_info *ti = __get_raw_thread(i);

		if (ti->tid == tid) {
			return ti;
		}
	}

	return NULL;
}

struct __thread_info *__get_current_thread(void)
{
	return __cpu_get_thread(__cpu_get_current());
}

pid_t __get_tid(void)
{
	struct __thread_info *ti = __get_current_thread();

	return ti->tid;
}
