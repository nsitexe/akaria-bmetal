/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DEVICE_H_
#define BAREMETAL_CRT_DEVICE_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/io.h>
#include <bmetal/sys/assert.h>
#include <bmetal/sys/errno.h>

#define __dev_err(dev, fmt, ...)     printk("%s: " fmt, __device_get_name(dev), ##__VA_ARGS__)
#define __dev_warn(dev, fmt, ...)    printk("%s: " fmt, __device_get_name(dev), ##__VA_ARGS__)
#define __dev_info(dev, fmt, ...)    printk("%s: " fmt, __device_get_name(dev), ##__VA_ARGS__)
#define __dev_dbg(dev, fmt, ...)     printk("%s: " fmt, __device_get_name(dev), ##__VA_ARGS__)

#define PROP_REVSEQ    \
	63, 62, 61, 60, 59, 58, 57, 56, \
	55, 54, 53, 52, 51, 50, 49, 48, \
	47, 46, 45, 44, 43, 42, 41, 40, \
	39, 38, 37, 36, 35, 34, 33, 32, \
	31, 30, 29, 28, 27, 26, 25, 24, \
	23, 22, 21, 20, 19, 18, 17, 16, \
	15, 14, 13, 12, 11, 10,  9,  8, \
	 7,  6,  5,  4,  3,  2,  1,  0
#define ____PROP_VA_ARGS_N(    \
	      A1,  A2,  A3,  A4,  A5,  A6,  A7, \
	 A8,  A9, A10, A11, A12, A13, A14, A15, \
	A16, A17, A18, A19, A20, A21, A22, A23, \
	A24, A25, A26, A27, A28, A29, A30, A31, \
	A32, A33, A34, A35, A36, A37, A38, A39, \
	A40, A41, A42, A43, A44, A45, A46, A47, \
	A48, A49, A50, A51, A52, A53, A54, A55, \
	A56, A57, A58, A59, A60, A61, A62, A63, \
	N, ...)    N
#define __PROP_VA_ARGS_N(...)  ____PROP_VA_ARGS_N(__VA_ARGS__)
#define PROP_VA_ARGS_N(...)    __PROP_VA_ARGS_N(__VA_ARGS__, PROP_REVSEQ)
#define PROP(NAME, ...)        {NAME, PROP_VA_ARGS_N(__VA_ARGS__), {__VA_ARGS__}}

#define EVENT_HANDLED            1
#define EVENT_NOT_HANDLED        2

#define for_each_handler(x, head)    for (struct __event_handler *x = (head); x; x = x->hnd_next)

struct __event_handler;

typedef int (* __event_handler_func_t)(int event, struct __event_handler *hnd);

struct __event_handler {
	/* Set by driver */
	__event_handler_func_t func;
	void *priv;

	/* Set by framework */
	int event;
	struct __event_handler *hnd_next;
};

#define for_each_driver(x, head)    for (struct __driver *x = (head); x; x = x->drv_next)
#define for_each_device(x, head)    for (struct __device *x = (head); x; x = x->dev_next)
#define for_each_bus(x, head)       for (struct __bus *x = (head); x; x = x->bus_next)

#define IS_ERROR(r)    ((r) != 0 && (r) != -EAGAIN)

#define CHECK_PRIV_SIZE(typ, lim)    static_assert(sizeof(lim) >= sizeof(typ), "size of " #lim " is less than " #typ);
#define CHECK_ELEM_SIZE(vala, valb)    static_assert(ARRAY_OF(vala) == ARRAY_OF(valb), "elements of " #vala " is not equal " #valb);

struct __device;
struct __bus;

struct __driver {
	const char *type_vendor;
	const char *type_device;

	struct __driver *drv_next;
};

struct __bus_driver_ops {
	int (*add)(struct __bus *bus);
	int (*remove)(struct __bus *bus);
	void *(*mmap)(void *addr, uintptr_t length, int prot, int flags, struct __bus *bus, uintptr_t off);
};

struct __bus_driver {
	struct __driver base;

	const struct __bus_driver_ops *ops;
};

struct __device_driver_ops {
	int (*add)(struct __device *dev);
	int (*remove)(struct __device *dev);
	void *(*mmap)(void *addr, uintptr_t length, int prot, int flags, struct __device *dev, uintptr_t off);
	uint8_t (*read8)(struct __device *dev, uintptr_t off);
	uint16_t (*read16)(struct __device *dev, uintptr_t off);
	uint32_t (*read32)(struct __device *dev, uintptr_t off);
	uint64_t (*read64)(struct __device *dev, uintptr_t off);
	void (*write8)(struct __device *dev, uint8_t dat, uintptr_t off);
	void (*write16)(struct __device *dev, uint16_t dat, uintptr_t off);
	void (*write32)(struct __device *dev, uint32_t dat, uintptr_t off);
	void (*write64)(struct __device *dev, uint64_t dat, uintptr_t off);
};

struct __device_driver {
	struct __driver base;

	const struct __device_driver_ops *ops;
};

struct __device_config {
	const char *name;
	int count;
	uintptr_t val[32];
};

struct __device {
	const char *name;
	const char *type_vendor;
	const char *type_device;
	const struct __device_config *conf;

	/* Runtime info */
	const struct __driver *drv;
	int id;

	struct __bus *bus_parent;
	/* Set non-NULL if this device has an other bus */
	struct __bus *bus_child;
	/* Set non-NULL if this device has a sibling */
	struct __device *dev_next;

	/* I/O region */
	struct __io_map io_base;

	/* Virtual address of I/O region if memory mapped */
	void *virt;

	/* Private data area for each driver */
	void *priv;

	int probed:1;
	int failed:1;
};

struct __bus {
	const char *name;
	const char *type_vendor;
	const char *type_device;

	/* Runtime info */
	const struct __driver *drv;
	int id;

	struct __device *dev_parent;
	/* Set non-NULL if other devices are connected to this bus */
	struct __device *dev_child;
	/* Set non-NULL if this device has a sibling */
	struct __bus *bus_next;

	/* Private data area for each driver */
	void *priv;

	int probed:1;
	int failed:1;
};

static inline const struct __device_driver *__device_get_drv(const struct __device *dev)
{
	if (!dev) {
		return NULL;
	}

	return (const struct __device_driver *)dev->drv;
}

static inline const char *__device_get_name(const struct __device *dev)
{
	if (!dev) {
		return NULL;
	}

	return dev->name;
}

static inline const struct __bus_driver *__bus_get_drv(const struct __bus *bus)
{
	if (!bus) {
		return NULL;
	}

	return (const struct __bus_driver *)bus->drv;
}

static inline uint8_t __device_read8(struct __device *dev, uintptr_t off)
{
	if (dev->virt) {
		return __io_read8(dev->virt + off);
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read8) {
		return devdrv->ops->read8(dev, off);
	}

	return -1;
}

static inline uint16_t __device_read16(struct __device *dev, uintptr_t off)
{
	if (dev->virt) {
		return __io_read16(dev->virt + off);
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read16) {
		return devdrv->ops->read16(dev, off);
	}

	return -1;
}

static inline uint32_t __device_read32(struct __device *dev, uintptr_t off)
{
	if (dev->virt) {
		return __io_read32(dev->virt + off);
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read32) {
		return devdrv->ops->read32(dev, off);
	}

	return -1;
}

static inline uint64_t __device_read64(struct __device *dev, uintptr_t off)
{
	if (dev->virt) {
		return __io_read64(dev->virt + off);
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read64) {
		return devdrv->ops->read64(dev, off);
	}

	return -1;
}

static inline void __device_write8(struct __device *dev, uint8_t dat, uintptr_t off)
{
	if (dev->virt) {
		__io_write8(dat, dev->virt + off);
		return;
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write8) {
		devdrv->ops->write8(dev, dat, off);
		return;
	}
}

static inline void __device_write16(struct __device *dev, uint16_t dat, uintptr_t off)
{
	if (dev->virt) {
		__io_write16(dat, dev->virt + off);
		return;
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write16) {
		devdrv->ops->write16(dev, dat, off);
		return;
	}
}

static inline void __device_write32(struct __device *dev, uint32_t dat, uintptr_t off)
{
	if (dev->virt) {
		__io_write32(dat, dev->virt + off);
		return;
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write32) {
		devdrv->ops->write32(dev, dat, off);
		return;
	}
}

static inline void __device_write64(struct __device *dev, uint64_t dat, uintptr_t off)
{
	if (dev->virt) {
		__io_write64(dat, dev->virt + off);
		return;
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write64) {
		devdrv->ops->write64(dev, dat, off);
		return;
	}
}

int __driver_add(struct __driver *driver);
int __driver_remove(struct __driver *driver);

struct __device *__device_get_root(void);
int __device_probe_all(void);
int __device_add(struct __device *dev, struct __bus *parent);
int __device_remove(struct __device *dev);
int __device_alloc_event_handler(struct __device *dev, struct __event_handler **handler);
int __device_free_event_handler(struct __device *dev, struct __event_handler *handler);
int __device_add_event_handler(struct __device *dev, struct __event_handler *head, struct __event_handler *handler);
int __device_remove_event_handler(struct __device *dev, struct __event_handler *head, struct __event_handler *handler);
int __device_get_conf_length(struct __device *dev, const char *name, int *len);
int __device_read_conf_u32(struct __device *dev, const char *name, uint32_t *ptr, int index);
int __device_read_conf_u64(struct __device *dev, const char *name, uint64_t *ptr, int index);
int __device_read_conf_str(struct __device *dev, const char *name, const char **ptr, int index);

void *__device_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct __device *dev, uintptr_t off);

struct __bus *__bus_get_root(void);
int __bus_add(struct __bus *bus, struct __device *parent);
int __bus_remove(struct __bus *bus);
int __bus_find_device(struct __bus *bus, const char *name, struct __device **dev);

void *__bus_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct __bus *bus, uintptr_t off);

#endif /* BAREMETAL_CRT_DEVICE_H_ */
