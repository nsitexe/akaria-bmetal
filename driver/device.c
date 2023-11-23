/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/string.h>

static struct k_driver head = {
	.type_vendor = "none",
	.type_device = "none",
};

static struct k_device_driver drv_root_dev = {
	.base = {
		.type_vendor = "none",
		.type_device = "dev_root",
	},
};

static struct k_bus_driver drv_root_bus = {
	.base = {
		.type_vendor = "none",
		.type_device = "bus_root",
	},
};

static struct k_device dev_root = {
	.name = "root device",
	.type_vendor = "none",
	.type_device = "dev_root",
};

static struct k_bus bus_root = {
	.name = "root bus",
	.type_vendor = "none",
	.type_device = "bus_root",
};

static int probe_all_enabled = 0;

static int device_probe(struct k_device *dev);
static int bus_probe(struct k_bus *bus);

static int bus_attach_driver(struct k_bus *bus)
{
	struct k_driver *drv = NULL;

	for_each_driver (d, &head) {
		if (k_strcmp(d->type_vendor, bus->type_vendor) == 0 &&
		    k_strcmp(d->type_device, bus->type_device) == 0) {
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

static int device_attach_driver(struct k_device *dev)
{
	struct k_driver *drv = NULL;

	for_each_driver (d, &head) {
		if (k_strcmp(d->type_vendor, dev->type_vendor) == 0 &&
		    k_strcmp(d->type_device, dev->type_device) == 0) {
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

static int device_probe(struct k_device *dev)
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
		const struct k_device_driver *drv_dev = k_device_get_drv(dev);

		if (drv_dev && drv_dev->ops && drv_dev->ops->add) {
			r = drv_dev->ops->add(dev);
			if (r == 0) {
				dev->probed = 1;
			} else if (r == -EAGAIN) {
				/* Probe again in the next time */
			} else {
				dev->failed = 1;
			}
		} else {
			/* Always succeed */
			r = 0;
			dev->probed = 1;
		}
		if (r) {
			goto err_out;
		}
	}

	if (dev->probed && dev->bus_child) {
		r = bus_probe(dev->bus_child);
		if (r) {
			goto err_out;
		}
	}

	return 0;

err_out:
	return r;
}

static int bus_probe(struct k_bus *bus)
{
	int r = 0;

	if (!bus) {
		return -EINVAL;
	}

	if (!bus->drv) {
		bus_attach_driver(bus);
	}

	if (bus->drv && !bus->probed) {
		const struct k_bus_driver *drv_bus = k_bus_get_drv(bus);

		if (drv_bus && drv_bus->ops && drv_bus->ops->add) {
			r = drv_bus->ops->add(bus);
			if (r == 0) {
				bus->probed = 1;
			} else if (r == -EAGAIN) {
				/* Probe again in the next time */
			} else {
				bus->failed = 1;
			}
		} else {
			/* Always succeed */
			r = 0;
			bus->probed = 1;
		}
		if (r) {
			goto err_out;
		}
	}

	if (!bus->dev_child) {
		return 0;
	}

	for_each_device (dev, bus->dev_child) {
		int s = device_probe(dev);
		if (IS_ERROR(s)) {
			pri_warn("bus:%d probe: Failed to probe dev:%d '%s'.\n",
				bus->id, dev->id, "");
			r = s;
			break;
		} else if (s == -EAGAIN) {
			/* Do not break here, try to probe all devices */
			r = -EAGAIN;
		}
	}
	if (r) {
		goto err_out;
	}

	return 0;

err_out:
	return r;
}

struct k_device *k_device_get_root(void)
{
	return &dev_root;
}

int k_device_get_probe_all_enabled(void)
{
	return probe_all_enabled;
}

int k_device_set_probe_all_enabled(int en)
{
	probe_all_enabled = en;

	return 0;
}

int k_device_probe_all(void)
{
	if (!k_device_get_probe_all_enabled()) {
		return -EAGAIN;
	}

	return device_probe(&dev_root);
}

int k_driver_add(struct k_driver *driver)
{
	struct k_driver *drv = NULL;

	if (!driver->type_vendor || !driver->type_device) {
		pri_warn("driver_add: Please set vendor and device.\n");
		return -EINVAL;
	}

	for_each_driver (d, &head) {
		if (k_strcmp(d->type_vendor, driver->type_vendor) == 0 &&
		    k_strcmp(d->type_device, driver->type_device) == 0) {
			pri_warn("Driver '%s:%s' has already registered.\n",
				driver->type_vendor, driver->type_device);
			return -EINVAL;
		}

		drv = d;
	}

	drv->drv_next = driver;

	int r = k_device_probe_all();
	if (IS_ERROR(r)) {
		pri_warn("driver_add: Probe failed (%d).\n", r);
	}

	return 0;
}

int k_driver_remove(struct k_driver *driver)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}

int k_device_add(struct k_device *dev, struct k_bus *parent)
{
	if (!dev || !parent) {
		return -EINVAL;
	}
	if (!dev->name) {
		pri_warn("device_add: Please set name.\n");
		return -EINVAL;
	}

	if (dev->bus_parent) {
		/* TODO: autogenerate device name */
		pri_warn("Dev:%d '%s' has already added into bus:%d '%s'.\n",
			dev->id, "", dev->bus_parent->id, "");
		return -EINVAL;
	}

	if (!parent->dev_child) {
		/* First child */
		parent->dev_child = dev;
	} else {
		struct k_device *d = NULL;

		for_each_device (tmp, parent->dev_child) {
			d = tmp;
		}

		d->dev_next = dev;
	}

	dev->bus_parent = parent;

	int r = k_device_probe_all();
	if (IS_ERROR(r)) {
		pri_warn("dev:%d add: Probe failed (%d).\n", dev->id, r);
	}

	return 0;
}

int k_device_remove(struct k_device *dev)
{
	if (!dev) {
		return -EINVAL;
	}

	/* TODO: to be implemented */
	/* dev->bus_parent = 0; */

	return -ENOTSUP;
}

static int device_find_conf(struct k_device *dev, const char *name)
{
	if (!dev || !dev->conf || !name) {
		return -EINVAL;
	}

	for (int i = 0; dev->conf[i].name != NULL; i++) {
		if (k_strcasecmp(name, dev->conf[i].name) == 0) {
			return i;
		}
	}

	return -1;
}

int k_device_get_conf_length(struct k_device *dev, const char *name, int *len)
{
	int i = device_find_conf(dev, name);
	if (i < 0) {
		return -EINVAL;
	}

	if (len) {
		*len = dev->conf[i].count;
	}

	return 0;
}

int k_device_read_conf_u32(struct k_device *dev, const char *name, uint32_t *ptr, int index)
{
	int i = device_find_conf(dev, name);
	if (i < 0) {
		return -EINVAL;
	}
	if (index >= dev->conf[i].count) {
		pri_warn("conf '%s': index %d exceeds array size %d.\n",
			name, index, dev->conf[i].count);
		return -EINVAL;
	}

	if (ptr) {
		*ptr = dev->conf[i].val[index];
	}

	return 0;
}

int k_device_read_conf_u64(struct k_device *dev, const char *name, uint64_t *ptr, int index)
{
	int i = device_find_conf(dev, name);
	if (i < 0) {
		return -EINVAL;
	}
	if (index >= dev->conf[i].count) {
		pri_warn("conf '%s': index %d exceeds array size %d.\n",
			name, index, dev->conf[i].count);
		return -EINVAL;
	}

	if (ptr) {
		*ptr = dev->conf[i].val[index];
	}

	return 0;
}

int k_device_read_conf_str(struct k_device *dev, const char *name, const char **ptr, int index)
{
	int i = device_find_conf(dev, name);
	if (i < 0) {
		return -EINVAL;
	}
	if (index >= dev->conf[i].count) {
		pri_warn("conf '%s': index %d exceeds array size %d.\n",
			name, index, dev->conf[i].count);
		return -EINVAL;
	}

	if (ptr) {
		*ptr = (const char *)dev->conf[i].val[index];
	}

	return 0;
}

void *k_device_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct k_device *dev, uintptr_t off)
{
	if (!dev) {
		return K_IO_MAP_FAILED;
	}

	const struct k_bus_driver *busdrv = k_bus_get_drv(dev->bus_parent);

	if (!busdrv || !busdrv->ops || !busdrv->ops->mmap) {
		return (void *)off;
	}

	return busdrv->ops->mmap(addr, length, prot, flags, dev->bus_parent, off);
}

struct k_bus *k_bus_get_root(void)
{
	return &bus_root;
}

int k_bus_add(struct k_bus *bus, struct k_device *parent)
{
	if (!bus || !parent) {
		return -EINVAL;
	}
	if (!bus->name) {
		pri_warn("bus_add: Please set name.\n");
		return -EINVAL;
	}

	if (bus->dev_parent) {
		/* TODO: autogenerate device name */
		pri_warn("Bus:%d '%s' has already had Dev:%d.\n",
			bus->id, "", bus->dev_parent->id);
		return -EINVAL;
	}

	if (!parent->bus_child) {
		/* First child */
		parent->bus_child = bus;
	} else {
		struct k_bus *b = NULL;

		for_each_bus (tmp, parent->bus_child) {
			b = tmp;
		}

		b->bus_next = bus;
	}

	bus->dev_parent = parent;

	int r = k_device_probe_all();
	if (IS_ERROR(r)) {
		pri_warn("bus:%d add: Probe failed (%d).\n", bus->id, r);
	}

	return 0;
}

int k_bus_remove(struct k_bus *bus)
{
	if (!bus) {
		return -EINVAL;
	}

	/* TODO: to be implemented */

	/*bus->dev_parent = NULL;*/
	/*parent->bus_child = NULL;*/

	return -ENOTSUP;
}

int k_bus_find_device(struct k_bus *bus, const char *name, struct k_device **dev)
{
	if (!bus || !name) {
		return -EINVAL;
	}

	for_each_device (tmp, bus->dev_child) {
		if (k_strcasecmp(name, tmp->name) == 0) {
			if (!tmp->probed) {
				return -EAGAIN;
			} else {
				if (dev) {
					*dev = tmp;
				}
				return 0;
			}
		}

		if (tmp->probed && tmp->bus_child) {
			return k_bus_find_device(tmp->bus_child, name, dev);
		}
	}

	return -ENODEV;
}

void *k_bus_driver_mmap(void *addr, uintptr_t length, int prot, int flags, struct k_bus *bus, uintptr_t off)
{
	if (!bus) {
		return K_IO_MAP_FAILED;
	}

	const struct k_device_driver *devdrv = k_device_get_drv(bus->dev_parent);

	if (!devdrv || !devdrv->ops || !devdrv->ops->mmap) {
		return (void *)off;
	}

	return devdrv->ops->mmap(addr, length, prot, flags, bus->dev_parent, off);
}

static int bus_init(void)
{
	k_driver_add(&drv_root_bus.base);
	k_bus_add(&bus_root, &dev_root);
	k_driver_add(&drv_root_dev.base);

	return 0;
}

define_init_func(bus_init);
