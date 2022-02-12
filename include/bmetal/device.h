/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DEVICE_H_
#define BAREMETAL_CRT_DEVICE_H_

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/io.h>

#define IS_ERROR(r)    ((r) != 0 && (r) != -EAGAIN)

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
	void (*write8)(struct __device *dev, uint8_t dat, uintptr_t off);
	void (*write16)(struct __device *dev, uint16_t dat, uintptr_t off);
	void (*write32)(struct __device *dev, uint32_t dat, uintptr_t off);
};

struct __device_driver {
	struct __driver base;

	const struct __device_driver_ops *ops;
};

struct __device_config {
	const char *name;
	uintptr_t val;
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

static inline void __device_write8(struct __device *dev, uint8_t dat, uintptr_t off)
{
	if (dev->virt) {
		__io_write8(dat, dev->virt + off);
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write8) {
		return devdrv->ops->write8(dev, dat, off);
	}
}

static inline void __device_write16(struct __device *dev, uint16_t dat, uintptr_t off)
{
	if (dev->virt) {
		__io_write16(dat, dev->virt + off);
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write16) {
		return devdrv->ops->write16(dev, dat, off);
	}
}

static inline void __device_write32(struct __device *dev, uint32_t dat, uintptr_t off)
{
	if (dev->virt) {
		__io_write32(dat, dev->virt + off);
	}

	const struct __device_driver *devdrv = __device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write32) {
		return devdrv->ops->write32(dev, dat, off);
	}
}

int __driver_add(struct __driver *driver);
int __driver_remove(struct __driver *driver);

struct __device *__device_get_root(void);
int __device_probe_all(void);
int __device_add(struct __device *dev, struct __bus *parent);
int __device_remove(struct __device *dev);
int __device_read_conf_u32(struct __device *dev, const char *name, uint32_t *ptr);
int __device_read_conf_u64(struct __device *dev, const char *name, uint64_t *ptr);

void *__device_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct __device *dev, uintptr_t off);

struct __bus *__bus_get_root(void);
int __bus_add(struct __bus *bus, struct __device *parent);
int __bus_remove(struct __bus *bus);

void *__bus_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct __bus *bus, uintptr_t off);

#endif /* BAREMETAL_CRT_DEVICE_H_ */
