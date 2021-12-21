/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_THREAD_H_
#define BAREMETAL_CRT_THREAD_H_

#include <sys/types.h>
#include <unistd.h>

#include <bmetal/bmetal.h>
#include <bmetal/file.h>

struct __process_info {
	pid_t pid;
	struct __file_desc *fdset[CONFIG_MAX_FD];
};

struct __thread_info {
	pid_t tid;

	struct __process_info *pi;
};

struct __process_info *__get_current_process(void);
pid_t __get_pid(void);

struct __thread_info *__get_raw_thread(int n);
struct __thread_info *__get_thread(pid_t tid);
struct __thread_info *__get_current_thread(void);
int __set_current_thread(struct __thread_info *ti);
pid_t __get_tid(void);

#endif /* BAREMETAL_CRT_THREAD_H_ */
