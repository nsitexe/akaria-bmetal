/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_THREAD_H_
#define BAREMETAL_CRT_THREAD_H_

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>
#include <bmetal/file.h>
#include <bmetal/lock.h>
#include <bmetal/sys/types.h>

#if !defined(__ASSEMBLER__)

struct k_cpu_device;

struct k_proc_info {
	pid_t pid;
	int avail;
	struct k_spinlock lock;
	struct k_thread_info *leader;

	struct k_file_desc *fdset[CONFIG_MAX_FD];
};

struct k_thread_info {
	struct k_proc_info *pi;
	pid_t tid;
	int avail;
	int running;

	unsigned long flags;
	char *sp;
	void *robust_list;
	size_t robust_len;
	pid_t *ctid;
	pid_t *ptid;
	void *tls;
	k_arch_user_regs_t regs;

	struct k_cpu_device *cpu;
};

struct k_proc_info *k_proc_create(void);
struct k_proc_info *k_proc_get_current(void);
pid_t k_proc_get_pid(void);
struct k_thread_info *k_proc_get_leader(struct k_proc_info *pi);
int k_proc_set_leader(struct k_proc_info *pi, struct k_thread_info *ti);

void k_thread_idle_main(int leader);
struct k_thread_info *k_thread_create(struct k_proc_info *pi);
int k_thread_destroy(struct k_thread_info *ti);
int k_thread_run(struct k_thread_info *ti, struct k_cpu_device *cpu);
int k_thread_stop(struct k_thread_info *ti);
struct k_thread_info *k_thread_get_raw(int n);
struct k_thread_info *k_thread_get(pid_t tid);
struct k_thread_info *k_thread_get_current(void);
pid_t k_thread_get_tid(void);
int k_thread_context_switch(void);
int k_thread_context_switch_nolock(void);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_THREAD_H_ */
