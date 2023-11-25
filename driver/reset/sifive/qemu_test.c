/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/reset.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

#define REG_FINISH    0x00

#define FINISH_FAIL     0x3333
#define FINISH_PASS     0x5555
#define FINISH_RESET    0x7777

struct reset_sifive_qemu_test_priv {
};
CHECK_PRIV_SIZE_RESET(struct reset_sifive_qemu_test_priv);

static int reset_sifive_qemu_test_reboot(struct k_reset_device *reset)
{
	struct k_device *d = k_reset_to_dev(reset);

	k_device_write32(d, FINISH_RESET, REG_FINISH);

	return 0;
}

static int reset_sifive_qemu_test_shutdown(struct k_reset_device *reset)
{
	struct k_device *d = k_reset_to_dev(reset);

	k_device_write32(d, FINISH_PASS, REG_FINISH);

	return 0;
}

static int reset_sifive_qemu_test_power_off(struct k_reset_device *reset)
{
	struct k_device *d = k_reset_to_dev(reset);

	k_device_write32(d, FINISH_PASS, REG_FINISH);

	return 0;
}

static int reset_sifive_qemu_test_add(struct k_device *dev)
{
	//struct k_reset_device *reset = k_reset_from_dev(dev);
	struct reset_sifive_qemu_test_priv *priv = dev->priv;
	int r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	/* Registers */
	r = k_io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	return 0;
}

static int reset_sifive_qemu_test_remove(struct k_device *dev)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}

const static struct k_device_driver_ops sifive_qemu_test_dev_ops = {
	.add = reset_sifive_qemu_test_add,
	.remove = reset_sifive_qemu_test_remove,
	.mmap = k_device_driver_mmap,
};

const static struct k_reset_driver_ops sifive_qemu_test_reset_ops = {
	.reboot = reset_sifive_qemu_test_reboot,
	.shutdown = reset_sifive_qemu_test_shutdown,
	.power_off = reset_sifive_qemu_test_power_off,
};

static struct k_reset_driver sifive_qemu_test_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "qemu_test",
		},

		.ops = &sifive_qemu_test_dev_ops,
	},

	.ops = &sifive_qemu_test_reset_ops,
};

static int reset_sifive_qemu_test_init(void)
{
	k_reset_add_driver(&sifive_qemu_test_drv);

	return 0;
}
define_init_func(reset_sifive_qemu_test_init);
