/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_THREAD_H_
#define BAREMETAL_CRT_THREAD_H_

#include <sys/types.h>
#include <unistd.h>

#include <bmetal/bmetal.h>
#include <bmetal/file.h>
#include <bmetal/lock.h>

struct __cpu_device;

struct __proc_info {
	pid_t pid;
	int avail;
	struct __spinlock lock;

	struct __file_desc *fdset[CONFIG_MAX_FD];
};

struct __thread_info {
	struct __proc_info *pi;
	pid_t tid;
	int avail;
	int running;

	struct __cpu_device *cpu;
};

struct __proc_info *__proc_create(void);
struct __proc_info *__proc_get_current(void);
pid_t __proc_get_pid(void);

struct __thread_info *__thread_create(struct __proc_info *pi);
int __thread_destroy(struct __thread_info *ti);
int __thread_run(struct __thread_info *ti, struct __cpu_device *cpu);
int __thread_stop(struct __thread_info *ti);
struct __thread_info *__thread_get_raw(int n);
struct __thread_info *__thread_get(pid_t tid);
struct __thread_info *__thread_get_current(void);
pid_t __thread_get_tid(void);

#endif /* BAREMETAL_CRT_THREAD_H_ */
