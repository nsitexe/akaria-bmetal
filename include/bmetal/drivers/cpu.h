/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_CPU_H_
#define BAREMETAL_CRT_DRIVERS_CPU_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct __cpu_device;

struct __cpu_driver_ops {
	int (*wakeup)(struct __cpu_device *cpu);
	int (*sleep)(struct __cpu_device *cpu);
};

struct __cpu_driver {
	struct __device_driver base;

	const struct __cpu_driver_ops *ops;
};

struct __cpu_device {
	struct __device base;

	int id_cpu;
	int id_phys;
};

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

int __cpu_alloc_id(void);
struct __cpu_device *__cpu_get(int id);
int __cpu_set(int id, struct __cpu_device *cpu);
struct __cpu_device *__cpu_get_by_physical_id(int id_phys);

int __cpu_add_device(struct __cpu_device *cpu, struct __bus *parent);
int __cpu_remove_device(struct __cpu_device *cpu);

int __cpu_wakeup_all(void);

#endif /* BAREMETAL_CRT_DRIVERS_CPU_H_ */
