/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_CPU_H_
#define BAREMETAL_CRT_DRIVERS_CPU_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>
#include <bmetal/arch.h>
#include <bmetal/lock.h>
#include <bmetal/thread.h>

enum k_cpu_event {
	CPU_EVENT_NULL,
	CPU_EVENT_ON_WAKEUP,
	CPU_EVENT_ON_SLEEP,

	CPU_EVENT_INTR_EXT,
	CPU_EVENT_INTR_IPI,
	CPU_EVENT_INTR_TIMER,

	CPU_EVENT_MAX,
};

struct k_cpu_futex {
	int *uaddr;
	int val;
	int bitset;
	int wakeup;
};

struct k_cpu_device;

struct k_cpu_driver_ops {
	/* Run on any CPUs */
	int (*clean_range)(struct k_cpu_device *cpu, const void *start, size_t sz);
	int (*inv_range)(struct k_cpu_device *cpu, const void *start, size_t sz);
	int (*flush_range)(struct k_cpu_device *cpu, const void *start, size_t sz);
	int (*wakeup)(struct k_cpu_device *cpu);
	int (*sleep)(struct k_cpu_device *cpu);

	/* Run on each CPU */
	int (*on_wakeup)(struct k_cpu_device *cpu);
	int (*on_sleep)(struct k_cpu_device *cpu);
	int (*raise_ipi)(struct k_cpu_device *cpu, struct k_cpu_device *dest, void *arg);
};

struct k_cpu_driver {
	struct __device_driver base;

	const struct k_cpu_driver_ops *ops;
};

struct k_cpu_device {
	struct __device base;

	int id_cpu;
	int id_phys;
	int running;
	k_arch_user_regs_t *regs;
	struct k_spinlock lock;
	struct k_thread_info *ti;
	struct k_thread_info *ti_idle;
	struct k_thread_info *ti_task;
	struct __event_handler handlers[CPU_EVENT_MAX];
	struct k_cpu_futex futex;

	int line_size_i;
	int line_size_d;
};

struct k_cpu_priv_max {
	char dummy[56];
};
typedef struct k_cpu_priv_max    k_cpu_priv_t;
#define CHECK_PRIV_SIZE_CPU(typ)    CHECK_PRIV_SIZE(typ, k_cpu_priv_t);

static inline const struct k_cpu_driver *k_cpu_get_drv(const struct k_cpu_device *cpu)
{
	if (!cpu) {
		return NULL;
	}

	return (const struct k_cpu_driver *)cpu->base.drv;
}

static inline struct __device *k_cpu_to_dev(struct k_cpu_device *cpu)
{
	if (!cpu) {
		return NULL;
	}

	return &cpu->base;
}

static inline struct k_cpu_device *k_cpu_from_dev(struct __device *dev)
{
	return (struct k_cpu_device *)dev;
}

static inline int k_cpu_add_driver(struct k_cpu_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int k_cpu_remove_driver(struct k_cpu_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

int k_cpu_get_id(struct k_cpu_device *cpu);
int k_cpu_get_id_phys(struct k_cpu_device *cpu);
int k_cpu_get_running(struct k_cpu_device *cpu);
void k_cpu_set_running(struct k_cpu_device *cpu, int r);

k_arch_user_regs_t *k_cpu_get_user_regs(struct k_cpu_device *cpu);
void k_cpu_set_user_regs(struct k_cpu_device *cpu, k_arch_user_regs_t *regs);
k_arch_user_regs_t *k_cpu_get_current_user_regs(void);
void k_cpu_set_current_user_regs(k_arch_user_regs_t *regs);

int k_cpu_lock(struct k_cpu_device *cpu);
int k_cpu_unlock(struct k_cpu_device *cpu);
struct k_thread_info *k_cpu_get_thread(struct k_cpu_device *cpu);
void k_cpu_set_thread(struct k_cpu_device *cpu, struct k_thread_info *ti);
struct k_thread_info *k_cpu_get_thread_idle(struct k_cpu_device *cpu);
void k_cpu_set_thread_idle(struct k_cpu_device *cpu, struct k_thread_info *ti);
struct k_thread_info *k_cpu_get_thread_task(struct k_cpu_device *cpu);
void k_cpu_set_thread_task(struct k_cpu_device *cpu, struct k_thread_info *ti);

int k_cpu_alloc_id(void);
struct k_cpu_device *k_cpu_get(int id);
int k_cpu_set(int id, struct k_cpu_device *cpu);
struct k_cpu_device *k_cpu_get_by_physical_id(int id_phys);
int k_cpu_get_current_id_phys(void);
struct k_cpu_device *k_cpu_get_current(void);

int k_cpu_add_device(struct k_cpu_device *cpu, struct __bus *parent);
int k_cpu_remove_device(struct k_cpu_device *cpu);
int k_cpu_cache_get_line_size_i(struct k_cpu_device *cpu);
void k_cpu_cache_set_line_size_i(struct k_cpu_device *cpu, int sz);
int k_cpu_cache_get_line_size_d(struct k_cpu_device *cpu);
void k_cpu_cache_set_line_size_d(struct k_cpu_device *cpu, int sz);
int k_cpu_cache_clean_range(struct k_cpu_device *cpu, const void *start, size_t sz);
int k_cpu_cache_inv_range(struct k_cpu_device *cpu, const void *start, size_t sz);
int k_cpu_cache_flush_range(struct k_cpu_device *cpu, const void *start, size_t sz);
int k_cpu_add_handler(struct k_cpu_device *cpu, enum k_cpu_event ev, struct __event_handler *hnd);
int k_cpu_remove_handler(struct k_cpu_device *cpu, enum k_cpu_event ev, struct __event_handler *hnd);
int k_cpu_wakeup(struct k_cpu_device *cpu);
int k_cpu_sleep(struct k_cpu_device *cpu);
int k_cpu_wakeup_all(void);
int k_cpu_sleep_all(void);
int k_cpu_on_wakeup(void);
int k_cpu_on_sleep(void);
int k_cpu_wait_interrupt(void);
int k_cpu_raise_ipi(struct k_cpu_device *dest, void *arg);
int k_cpu_futex_wait(int *uaddr, int val, int bitset);
int k_cpu_futex_wake(int *uaddr, int val, int bitset);

int k_cpu_get_cpu_from_config(struct __device *dev, int index, struct k_cpu_device **cpu);

#endif /* BAREMETAL_CRT_DRIVERS_CPU_H_ */
