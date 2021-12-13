/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/thread.h>
#include <bmetal/printk.h>

static struct __process_info __pi;
static struct __thread_info __ti[CONFIG_NUM_CORES];

struct __process_info *__get_current_process(void)
{
	return &__pi;
}

pid_t __get_pid(void)
{
	struct __process_info *pi = __get_current_process();

	return pi->pid;
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
	struct __thread_info *ti;

	__asm volatile("mv %0, tp" : "=r"(ti));

	return ti;
}

int __set_current_thread(struct __thread_info *ti)
{
	if (!ti) {
		printk("set_current_thread: invalid thread.\n");
		return -1;
	}

	__asm volatile("mv tp, %0" : : "r"(ti));

	return 0;
}

pid_t __get_tid(void)
{
	struct __thread_info *ti = __get_current_thread();

	return ti->tid;
}
