/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <string.h>

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>

static struct __driver head = {
	.type_vendor = "none",
	.type_device = "none",
};

static struct __device_driver drv_root_dev = {
	.base = {
		.type_vendor = "none",
		.type_device = "dev_root",
	},
};

static struct __bus_driver drv_root_bus = {
	.base = {
		.type_vendor = "none",
		.type_device = "bus_root",
	},
};

static struct __device dev_root = {
	.name = "root device",
	.type_vendor = "none",
	.type_device = "dev_root",
};

static struct __bus bus_root = {
	.name = "root bus",
	.type_vendor = "none",
	.type_device = "bus_root",
};

static int device_probe(struct __device *dev);
static int bus_probe(struct __bus *bus);

static int bus_attach_driver(struct __bus *bus)
{
	struct __driver *drv = NULL;

	for_each_driver (d, &head) {
		if (strcmp(d->type_vendor, bus->type_vendor) == 0 &&
		    strcmp(d->type_device, bus->type_device) == 0) {
			drv = d;
			break;
		}
	}
	if (drv == &head || !drv) {
		return -EAGAIN;
	}

	bus->drv = drv;

	return 0;
}


static int device_attach_driver(struct __device *dev)
{
	struct __driver *drv = NULL;

	for_each_driver (d, &head) {
		if (strcmp(d->type_vendor, dev->type_vendor) == 0 &&
		    strcmp(d->type_device, dev->type_device) == 0) {
			drv = d;
			break;
		}
	}
	if (drv == &head || !drv) {
		return -EAGAIN;
	}

	dev->drv = drv;

	return 0;
}

static int device_probe(struct __device *dev)
{
	int r = 0;

	if (!dev) {
		return -EINVAL;
	}

	if (!dev->drv) {
		device_attach_driver(dev);
	}
	if (!dev->drv) {
		return -EAGAIN;
	}

	if (dev->drv && !dev->probed) {
		const struct __device_driver *drv_dev = __device_get_drv(dev);

		if (drv_dev && drv_dev->ops && drv_dev->ops->add) {
			r = drv_dev->ops->add(dev);
			if (r == 0) {
				dev->probed = 1;
			} else if (r == -EAGAIN) {
				/* Probe again in the next time */
			} else if (r != 0) {
				dev->failed = 1;
			}
		} else {
			/* Always succeed */
			dev->probed = 1;
		}
	}
	if (!dev->probed) {
		goto err_out;
	}

	if (dev->probed && dev->bus_child) {
		bus_probe(dev->bus_child);
	}

	return 0;

err_out:
	return r;
}

static int bus_probe(struct __bus *bus)
{
	int r = 0;

	if (!bus) {
		return -EINVAL;
	}

	if (!bus->drv) {
		bus_attach_driver(bus);
	}

	if (bus->drv && !bus->probed) {
		const struct __bus_driver *drv_bus = __bus_get_drv(bus);

		if (drv_bus && drv_bus->ops && drv_bus->ops->add) {
			r = drv_bus->ops->add(bus);
			if (r == 0) {
				bus->probed = 1;
			} else if (r == -EAGAIN) {
				/* Probe again in the next time */
			} else if (r != 0) {
				bus->failed = 1;
			}
		} else {
			/* Always succeed */
			bus->probed = 1;
		}
	}
	if (!bus->probed) {
		goto err_out;
	}

	if (!bus->dev_child) {
		return 0;
	}

	for_each_device (dev, bus->dev_child) {
		int r = device_probe(dev);
		if (IS_ERROR(r)) {
			printk("bus:%d probe: Failed to probe dev:%d '%s'.",
				bus->id, dev->id, "");
		}
	}

	return 0;

err_out:
	return r;
}

struct __device *__device_get_root(void)
{
	return &dev_root;
}

int __device_probe_all(void)
{
	return device_probe(&dev_root);
}

int __driver_add(struct __driver *driver)
{
	struct __driver *drv = NULL;

	if (!driver->type_vendor || !driver->type_device) {
		printk("driver_add: Please set vendor and device.\n");
		return -EINVAL;
	}

	for_each_driver (d, &head) {
		if (strcmp(d->type_vendor, driver->type_vendor) == 0 &&
		    strcmp(d->type_device, driver->type_device) == 0) {
			printk("Driver '%s:%s' has already registered.\n",
				driver->type_vendor, driver->type_device);
			return -EINVAL;
		}

		drv = d;
	}

	drv->drv_next = driver;

	int r = __device_probe_all();
	if (IS_ERROR(r)) {
		printk("driver_add: Probe failed (%d).\n", r);
	}

	return 0;
}

int __driver_remove(struct __driver *driver)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}

int __device_add(struct __device *dev, struct __bus *parent)
{
	if (!dev || !parent) {
		return -EINVAL;
	}
	if (!dev->name) {
		printk("device_add: Please set name.\n");
		return -EINVAL;
	}

	if (dev->bus_parent) {
		/* TODO: autogenerate device name */
		printk("Dev:%d '%s' has already added into bus:%d '%s'.\n",
			dev->id, "", dev->bus_parent->id, "");
		return -EINVAL;
	}

	if (!parent->dev_child) {
		/* First child */
		parent->dev_child = dev;
	} else {
		struct __device *d = NULL;

		for_each_device (tmp, parent->dev_child) {
			d = tmp;
		}

		d->dev_next = dev;
	}

	dev->bus_parent = parent;

	int r = __device_probe_all();
	if (IS_ERROR(r)) {
		printk("dev:%d add: Probe failed (%d).\n", dev->id, r);
	}

	return 0;
}

int __device_remove(struct __device *dev)
{
	if (!dev) {
		return -EINVAL;
	}

	/* TODO: to be implemented */
	/* dev->bus_parent = 0; */

	return -ENOTSUP;
}

static int device_find_conf(struct __device *dev, const char *name)
{
	if (!dev || !dev->conf || !name) {
		return -EINVAL;
	}

	for (int i = 0; dev->conf[i].name != NULL; i++) {
		if (strcasecmp(name, dev->conf[i].name) == 0) {
			return i;
		}
	}

	return -1;
}

int __device_read_conf_u32(struct __device *dev, const char *name, uint32_t *ptr, int index)
{
	int i = device_find_conf(dev, name);
	if (i < 0) {
		return -EINVAL;
	}
	if (index >= dev->conf[i].count) {
		printk("conf '%s': index %d exceeds array size %d.\n",
			name, index, dev->conf[i].count);
		return -EINVAL;
	}

	if (ptr) {
		*ptr = dev->conf[i].val[index];
	}

	return 0;
}

int __device_read_conf_u64(struct __device *dev, const char *name, uint64_t *ptr, int index)
{
	int i = device_find_conf(dev, name);
	if (i < 0) {
		return -EINVAL;
	}
	if (index >= dev->conf[i].count) {
		printk("conf '%s': index %d exceeds array size %d.\n",
			name, index, dev->conf[i].count);
		return -EINVAL;
	}

	if (ptr) {
		*ptr = dev->conf[i].val[index];
	}

	return 0;
}

int __device_read_conf_str(struct __device *dev, const char *name, const char **ptr, int index)
{
	int i = device_find_conf(dev, name);
	if (i < 0) {
		return -EINVAL;
	}
	if (index >= dev->conf[i].count) {
		printk("conf '%s': index %d exceeds array size %d.\n",
			name, index, dev->conf[i].count);
		return -EINVAL;
	}

	if (ptr) {
		*ptr = (const char *)dev->conf[i].val[index];
	}

	return 0;
}

void *__device_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct __device *dev, uintptr_t off)
{
	if (!dev) {
		return __IO_MAP_FAILED;
	}

	const struct __bus_driver *busdrv = __bus_get_drv(dev->bus_parent);

	if (!busdrv || !busdrv->ops || !busdrv->ops->mmap) {
		return (void *)off;
	}

	return busdrv->ops->mmap(addr, length, prot, flags, dev->bus_parent, off);
}

struct __bus *__bus_get_root(void)
{
	return &bus_root;
}

int __bus_add(struct __bus *bus, struct __device *parent)
{
	if (!bus || !parent) {
		return -EINVAL;
	}
	if (!bus->name) {
		printk("bus_add: Please set name.\n");
		return -EINVAL;
	}

	if (bus->dev_parent) {
		/* TODO: autogenerate device name */
		printk("Bus:%d '%s' has already had Dev:%d.\n",
			bus->id, "", bus->dev_parent->id);
		return -EINVAL;
	}

	if (!parent->bus_child) {
		/* First child */
		parent->bus_child = bus;
	} else {
		struct __bus *b = NULL;

		for_each_bus (tmp, parent->bus_child) {
			b = tmp;
		}

		b->bus_next = bus;
	}

	bus->dev_parent = parent;

	int r = __device_probe_all();
	if (IS_ERROR(r)) {
		printk("bus:%d add: Probe failed (%d).\n", bus->id, r);
	}

	return 0;
}

int __bus_remove(struct __bus *bus)
{
	if (!bus) {
		return -EINVAL;
	}

	/* TODO: to be implemented */

	/*bus->dev_parent = NULL;*/
	/*parent->bus_child = NULL;*/

	return -ENOTSUP;
}

void *__bus_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct __bus *bus, uintptr_t off)
{
	if (!bus) {
		return __IO_MAP_FAILED;
	}

	const struct __device_driver *devdrv = __device_get_drv(bus->dev_parent);

	if (!devdrv || !devdrv->ops || !devdrv->ops->mmap) {
		return (void *)off;
	}

	return devdrv->ops->mmap(addr, length, prot, flags, bus->dev_parent, off);
}

static int bus_init(void)
{
	__driver_add(&drv_root_bus.base);
	__bus_add(&bus_root, &dev_root);
	__driver_add(&drv_root_dev.base);

	return 0;
}

define_init_func(bus_init);
