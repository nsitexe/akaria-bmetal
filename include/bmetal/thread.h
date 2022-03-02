/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_THREAD_H_
#define BAREMETAL_CRT_THREAD_H_

#include <sys/types.h>
#include <unistd.h>

#include <bmetal/bmetal.h>
#include <bmetal/file.h>
#include <bmetal/lock.h>

struct __cpu_device;

struct __process_info {
	pid_t pid;
	int avail;
	struct __spinlock lock;

	struct __file_desc *fdset[CONFIG_MAX_FD];
};

struct __thread_info {
	struct __process_info *pi;
	pid_t tid;
	int avail;
	int running;

	struct __cpu_device *cpu;
};

struct __process_info *__create_process(void);
struct __process_info *__get_current_process(void);
pid_t __get_pid(void);

struct __thread_info *__create_thread(struct __process_info *pi);
int __destroy_thread(struct __thread_info *ti);
int __run_thread(struct __thread_info *ti, struct __cpu_device *cpu);
int __stop_thread(struct __thread_info *ti);
struct __thread_info *__get_raw_thread(int n);
struct __thread_info *__get_thread(pid_t tid);
struct __thread_info *__get_current_thread(void);
pid_t __get_tid(void);

#endif /* BAREMETAL_CRT_THREAD_H_ */
