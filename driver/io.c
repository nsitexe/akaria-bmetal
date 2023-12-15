/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/io.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

int k_io_mmap_device(void *addr, struct k_device *dev)
{
	uint64_t reg, size;
	int r;

	r = k_device_read_conf_u64(dev, "reg", &reg, 0);
	if (r) {
		k_pri_warn("io_mmap_device: config 'reg' is not found.\n");
		return -EINVAL;
	}

	r = k_device_read_conf_u64(dev, "reg-size", &size, 0);
	if (r) {
		k_pri_warn("io_mmap_device: config 'reg-size' is not found.\n");
		return -EINVAL;
	}
	dev->io_base.addr = reg;
	dev->io_base.size = size;

	const struct k_device_driver *devdrv = k_device_get_drv(dev);

	if (!devdrv || !devdrv->ops || !devdrv->ops->mmap) {
		dev->virt = K_IO_MAP_FAILED;
	} else {
		dev->virt = devdrv->ops->mmap(addr, size, 0, 0, dev, reg);
	}
	if (dev->virt == K_IO_MAP_FAILED) {
		dev->virt = NULL;
		k_pri_warn("io_mmap_device: map '0x%08"PRIx64"' failed.\n", reg);
		return -EIO;
	}

	return 0;
}

int k_io_munmap_device(void *addr, struct k_device *dev)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}
