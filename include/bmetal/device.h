/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DEVICE_H_
#define BAREMETAL_CRT_DEVICE_H_

#if !defined(__ASSEMBLER__)
#include <stddef.h>
#include <stdint.h>
#endif /* !__ASSEMBLER__ */

#include <bmetal/bmetal.h>
#include <bmetal/io.h>
#include <bmetal/sys/assert.h>
#include <bmetal/sys/errno.h>

#if !defined(__ASSEMBLER__)

#define __dev_err(dev, fmt, ...)     pri_err("%s: " fmt, k_device_get_name(dev), ##__VA_ARGS__)
#define __dev_warn(dev, fmt, ...)    pri_warn("%s: " fmt, k_device_get_name(dev), ##__VA_ARGS__)
#define __dev_info(dev, fmt, ...)    pri_info("%s: " fmt, k_device_get_name(dev), ##__VA_ARGS__)
#define __dev_dbg(dev, fmt, ...)     pri_dbg("%s: " fmt, k_device_get_name(dev), ##__VA_ARGS__)

#define PROP_REVSEQ    \
	109, 108, 107, 106, 105, 104, 103, 102, 101, 100, \
	99,  98,  97,  96,  95,  94,  93,  92,  91,  90, \
	89,  88,  87,  86,  85,  84,  83,  82,  81,  80, \
	79,  78,  77,  76,  75,  74,  73,  72,  71,  70, \
	69,  68,  67,  66,  65,  64,  63,  62,  61,  60, \
	59,  58,  57,  56,  55,  54,  53,  52,  51,  50, \
	49,  48,  47,  46,  45,  44,  43,  42,  41,  40, \
	39,  38,  37,  36,  35,  34,  33,  32,  31,  30, \
	29,  28,  27,  26,  25,  24,  23,  22,  21,  20, \
	19,  18,  17,  16,  15,  14,  13,  12,  11,  10, \
	9,   8,	  7,   6,   5,   4,   3,   2,   1,   0
#define ____PROP_VA_ARGS_N(    \
	      A1,   A2,   A3,   A4,   A5,   A6,   A7,   A8,   A9, \
	A10,  A11,  A12,  A13,  A14,  A15,  A16,  A17,  A18,  A19, \
	A20,  A21,  A22,  A23,  A24,  A25,  A26,  A27,  A28,  A29, \
	A30,  A31,  A32,  A33,  A34,  A35,  A36,  A37,  A38,  A39, \
	A40,  A41,  A42,  A43,  A44,  A45,  A46,  A47,  A48,  A49, \
	A50,  A51,  A52,  A53,  A54,  A55,  A56,  A57,  A58,  A59, \
	A60,  A61,  A62,  A63,  A64,  A65,  A66,  A67,  A68,  A69, \
	A70,  A71,  A72,  A73,  A74,  A75,  A76,  A77,  A78,  A79, \
	A80,  A81,  A82,  A83,  A84,  A85,  A86,  A87,  A88,  A89, \
	A90,  A91,  A92,  A93,  A94,  A95,  A96,  A97,  A98,  A99, \
	A100, A101, A102, A103, A104, A105, A106, A107, A108, A109, \
	N, ...)    N
#define __PROP_VA_ARGS_N(...)  ____PROP_VA_ARGS_N(__VA_ARGS__)
#define PROP_VA_ARGS_N(...)    __PROP_VA_ARGS_N(__VA_ARGS__, PROP_REVSEQ)
#define PROP(NAME, ...)        {NAME, PROP_VA_ARGS_N(__VA_ARGS__), {__VA_ARGS__}}

#define for_each_driver(x, head)    for (struct k_driver *x = (head); x; x = x->drv_next)
#define for_each_device(x, head)    for (struct k_device *x = (head); x; x = x->dev_next)
#define for_each_bus(x, head)       for (struct k_bus *x = (head); x; x = x->bus_next)

#define IS_ERROR(r)    ((r) != 0 && (r) != -EAGAIN)

#define CHECK_PRIV_SIZE(typ, lim)    static_assert(sizeof(lim) >= sizeof(typ), "size of " #lim " is less than " #typ);
#define CHECK_ELEM_SIZE(vala, valb)    static_assert(ARRAY_OF(vala) == ARRAY_OF(valb), "elements of " #vala " is not equal " #valb);

struct k_device;
struct k_bus;

struct k_driver {
	const char *type_vendor;
	const char *type_device;

	struct k_driver *drv_next;
};

struct k_bus_driver_ops {
	int (*add)(struct k_bus *bus);
	int (*remove)(struct k_bus *bus);
	void *(*mmap)(void *addr, uintptr_t length, int prot, int flags, struct k_bus *bus, uintptr_t off);
};

struct k_bus_driver {
	struct k_driver base;

	const struct k_bus_driver_ops *ops;
};

struct k_device_driver_ops {
	int (*add)(struct k_device *dev);
	int (*remove)(struct k_device *dev);
	void *(*mmap)(void *addr, uintptr_t length, int prot, int flags, struct k_device *dev, uintptr_t off);
	uint8_t (*read8)(struct k_device *dev, uintptr_t off);
	uint16_t (*read16)(struct k_device *dev, uintptr_t off);
	uint32_t (*read32)(struct k_device *dev, uintptr_t off);
	uint64_t (*read64)(struct k_device *dev, uintptr_t off);
	void (*write8)(struct k_device *dev, uint8_t dat, uintptr_t off);
	void (*write16)(struct k_device *dev, uint16_t dat, uintptr_t off);
	void (*write32)(struct k_device *dev, uint32_t dat, uintptr_t off);
	void (*write64)(struct k_device *dev, uint64_t dat, uintptr_t off);
};

struct k_device_driver {
	struct k_driver base;

	const struct k_device_driver_ops *ops;
};

struct k_device_config {
	const char *name;
	int count;
	uintptr_t val[32];
};

struct k_device {
	const char *name;
	const char *type_vendor;
	const char *type_device;
	const struct k_device_config *conf;

	/* Runtime info */
	const struct k_driver *drv;
	int id;

	struct k_bus *bus_parent;
	/* Set non-NULL if this device has an other bus */
	struct k_bus *bus_child;
	/* Set non-NULL if this device has a sibling */
	struct k_device *dev_next;

	/* I/O region */
	struct k_io_map io_base;

	/* Virtual address of I/O region if memory mapped */
	void *virt;

	/* Private data area for each driver */
	void *priv;

	unsigned int probed:1;
	unsigned int failed:1;
};

struct k_bus {
	const char *name;
	const char *type_vendor;
	const char *type_device;

	/* Runtime info */
	const struct k_driver *drv;
	int id;

	struct k_device *dev_parent;
	/* Set non-NULL if other devices are connected to this bus */
	struct k_device *dev_child;
	/* Set non-NULL if this device has a sibling */
	struct k_bus *bus_next;

	/* Private data area for each driver */
	void *priv;

	unsigned int probed:1;
	unsigned int failed:1;
};

static inline const struct k_device_driver *k_device_get_drv(const struct k_device *dev)
{
	if (!dev) {
		return NULL;
	}

	return (const struct k_device_driver *)dev->drv;
}

static inline const char *k_device_get_name(const struct k_device *dev)
{
	if (!dev) {
		return NULL;
	}

	return dev->name;
}

static inline const struct k_bus_driver *k_bus_get_drv(const struct k_bus *bus)
{
	if (!bus) {
		return NULL;
	}

	return (const struct k_bus_driver *)bus->drv;
}

static inline uint8_t k_device_read8(struct k_device *dev, uintptr_t off)
{
	if (dev->virt) {
		return k_io_read8(dev->virt + off);
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read8) {
		return devdrv->ops->read8(dev, off);
	}

	return -1;
}

static inline uint16_t k_device_read16(struct k_device *dev, uintptr_t off)
{
	if (dev->virt) {
		return k_io_read16(dev->virt + off);
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read16) {
		return devdrv->ops->read16(dev, off);
	}

	return -1;
}

static inline uint32_t k_device_read32(struct k_device *dev, uintptr_t off)
{
	if (dev->virt) {
		return k_io_read32(dev->virt + off);
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read32) {
		return devdrv->ops->read32(dev, off);
	}

	return -1;
}

static inline uint64_t k_device_read64(struct k_device *dev, uintptr_t off)
{
	if (dev->virt) {
		return k_io_read64(dev->virt + off);
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->read64) {
		return devdrv->ops->read64(dev, off);
	}

	return -1;
}

static inline void k_device_write8(struct k_device *dev, uint8_t dat, uintptr_t off)
{
	if (dev->virt) {
		k_io_write8(dat, dev->virt + off);
		return;
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write8) {
		devdrv->ops->write8(dev, dat, off);
		return;
	}
}

static inline void k_device_write16(struct k_device *dev, uint16_t dat, uintptr_t off)
{
	if (dev->virt) {
		k_io_write16(dat, dev->virt + off);
		return;
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write16) {
		devdrv->ops->write16(dev, dat, off);
		return;
	}
}

static inline void k_device_write32(struct k_device *dev, uint32_t dat, uintptr_t off)
{
	if (dev->virt) {
		k_io_write32(dat, dev->virt + off);
		return;
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write32) {
		devdrv->ops->write32(dev, dat, off);
		return;
	}
}

static inline void k_device_write64(struct k_device *dev, uint64_t dat, uintptr_t off)
{
	if (dev->virt) {
		k_io_write64(dat, dev->virt + off);
		return;
	}

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (devdrv && devdrv->ops && devdrv->ops->write64) {
		devdrv->ops->write64(dev, dat, off);
		return;
	}
}

int k_driver_add(struct k_driver *driver);
int k_driver_remove(struct k_driver *driver);

struct k_device *k_device_get_root(void);
int k_device_get_probe_all_enabled(void);
int k_device_set_probe_all_enabled(int en);
int k_device_probe_all(void);
int k_device_add(struct k_device *dev, struct k_bus *parent);
int k_device_remove(struct k_device *dev);
int k_device_get_conf_length(struct k_device *dev, const char *name, int *len);
int k_device_read_conf_u32(struct k_device *dev, const char *name, uint32_t *ptr, int index);
int k_device_read_conf_u64(struct k_device *dev, const char *name, uint64_t *ptr, int index);
int k_device_read_conf_str(struct k_device *dev, const char *name, const char **ptr, int index);

void *k_device_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct k_device *dev, uintptr_t off);

struct k_bus *k_bus_get_root(void);
int k_bus_add(struct k_bus *bus, struct k_device *parent);
int k_bus_remove(struct k_bus *bus);
int k_bus_find_device(struct k_bus *bus, const char *name, struct k_device **dev);

void *k_bus_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct k_bus *bus, uintptr_t off);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_DEVICE_H_ */
