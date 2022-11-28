/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_CPU_H_
#define BAREMETAL_CRT_DRIVERS_CPU_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>
#include <bmetal/arch.h>
#include <bmetal/lock.h>
#include <bmetal/thread.h>

enum __cpu_event {
	CPU_EVENT_ON_WAKEUP,
	CPU_EVENT_ON_SLEEP,

	CPU_EVENT_MAX,
};

struct __cpu_futex {
	int *uaddr;
	int bitset;
	int wakeup;
};

struct __cpu_device;

struct __cpu_driver_ops {
	/* Run on any CPUs */
	int (*clean_range)(struct __cpu_device *cpu, const void *start, size_t sz);
	int (*inv_range)(struct __cpu_device *cpu, const void *start, size_t sz);
	int (*flush_range)(struct __cpu_device *cpu, const void *start, size_t sz);
	int (*wakeup)(struct __cpu_device *cpu);
	int (*sleep)(struct __cpu_device *cpu);

	/* Run on each CPU */
	int (*on_wakeup)(struct __cpu_device *cpu);
	int (*on_sleep)(struct __cpu_device *cpu);
	int (*raise_ipi)(struct __cpu_device *cpu, struct __cpu_device *dest, void *arg);
};

struct __cpu_driver {
	struct __device_driver base;

	const struct __cpu_driver_ops *ops;
};

struct __cpu_device {
	struct __device base;

	int id_cpu;
	int id_phys;
	int running;
	__arch_user_regs_t *regs;
	struct __spinlock lock;
	struct __thread_info *ti;
	struct __thread_info *ti_idle;
	struct __thread_info *ti_task;
	struct __event_handler handlers[CPU_EVENT_MAX];
	struct __cpu_futex futex;

	int line_size_i;
	int line_size_d;
};

struct __cpu_priv_max {
	char dummy[56];
};
typedef struct __cpu_priv_max    __cpu_priv_t;
#define CHECK_PRIV_SIZE_CPU(typ)    CHECK_PRIV_SIZE(typ, __cpu_priv_t);

static inline const struct __cpu_driver *__cpu_get_drv(const struct __cpu_device *cpu)
{
	if (!cpu) {
		return NULL;
	}

	return (const struct __cpu_driver *)cpu->base.drv;
}

static inline struct __device *__cpu_to_dev(struct __cpu_device *cpu)
{
	if (!cpu) {
		return NULL;
	}

	return &cpu->base;
}

static inline struct __cpu_device *__cpu_from_dev(struct __device *dev)
{
	return (struct __cpu_device *)dev;
}

static inline int __cpu_add_driver(struct __cpu_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int __cpu_remove_driver(struct __cpu_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

int __cpu_get_id(struct __cpu_device *cpu);
int __cpu_get_id_phys(struct __cpu_device *cpu);
int __cpu_get_running(struct __cpu_device *cpu);
void __cpu_set_running(struct __cpu_device *cpu, int r);

__arch_user_regs_t *__cpu_get_user_regs(struct __cpu_device *cpu);
void __cpu_set_user_regs(struct __cpu_device *cpu, __arch_user_regs_t *regs);
__arch_user_regs_t *__cpu_get_current_user_regs(void);
void __cpu_set_current_user_regs(__arch_user_regs_t *regs);

int __cpu_lock(struct __cpu_device *cpu);
int __cpu_unlock(struct __cpu_device *cpu);
struct __thread_info *__cpu_get_thread(struct __cpu_device *cpu);
void __cpu_set_thread(struct __cpu_device *cpu, struct __thread_info *ti);
struct __thread_info *__cpu_get_thread_idle(struct __cpu_device *cpu);
void __cpu_set_thread_idle(struct __cpu_device *cpu, struct __thread_info *ti);
struct __thread_info *__cpu_get_thread_task(struct __cpu_device *cpu);
void __cpu_set_thread_task(struct __cpu_device *cpu, struct __thread_info *ti);

int __cpu_alloc_id(void);
struct __cpu_device *__cpu_get(int id);
int __cpu_set(int id, struct __cpu_device *cpu);
struct __cpu_device *__cpu_get_by_physical_id(int id_phys);
struct __cpu_device *__cpu_get_current(void);

int __cpu_add_device(struct __cpu_device *cpu, struct __bus *parent);
int __cpu_remove_device(struct __cpu_device *cpu);
int __cpu_cache_get_line_size_i(struct __cpu_device *cpu);
void __cpu_cache_set_line_size_i(struct __cpu_device *cpu, int sz);
int __cpu_cache_get_line_size_d(struct __cpu_device *cpu);
void __cpu_cache_set_line_size_d(struct __cpu_device *cpu, int sz);
int __cpu_cache_clean_range(struct __cpu_device *cpu, const void *start, size_t sz);
int __cpu_cache_inv_range(struct __cpu_device *cpu, const void *start, size_t sz);
int __cpu_cache_flush_range(struct __cpu_device *cpu, const void *start, size_t sz);
int __cpu_add_handler(struct __cpu_device *cpu, enum __cpu_event ev, struct __event_handler *hnd);
int __cpu_remove_handler(struct __cpu_device *cpu, enum __cpu_event ev, struct __event_handler *hnd);
int __cpu_wakeup(struct __cpu_device *cpu);
int __cpu_sleep(struct __cpu_device *cpu);
int __cpu_wakeup_all(void);
int __cpu_sleep_all(void);
int __cpu_on_wakeup(void);
int __cpu_on_sleep(void);
int __cpu_wait_interrupt(void);
int __cpu_raise_ipi(struct __cpu_device *dest, void *arg);
int __cpu_futex_wait(int *uaddr, int val, int bitset);
int __cpu_futex_wake(int *uaddr, int val, int bitset);

int __cpu_get_cpu_from_config(struct __device *dev, int index, struct __cpu_device **cpu);

#endif /* BAREMETAL_CRT_DRIVERS_CPU_H_ */
