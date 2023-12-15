/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/clk.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>

int k_clk_add_device(struct k_clk_device *clk, struct k_bus *parent)
{
	int r;

	r = k_device_add(k_clk_to_dev(clk), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	return 0;
}

int k_clk_remove_device(struct k_clk_device *clk)
{
	return k_device_remove(k_clk_to_dev(clk));
}

int k_clk_enable(struct k_clk_device *clk, int index)
{
	const struct k_clk_driver *drv = k_clk_get_drv(clk);
	int r = -ENODEV;

	if (drv && drv->ops && drv->ops->enable) {
		r = drv->ops->enable(clk, index);
	}

	return r;
}

int k_clk_disable(struct k_clk_device *clk, int index)
{
	const struct k_clk_driver *drv = k_clk_get_drv(clk);
	int r = -ENODEV;

	if (drv && drv->ops && drv->ops->disable) {
		r = drv->ops->disable(clk, index);
	}

	return r;
}

int k_clk_get_frequency(struct k_clk_device *clk, int index, uint64_t *freq)
{
	const struct k_clk_driver *drv = k_clk_get_drv(clk);
	uint64_t buf;
	int r = -ENODEV;

	if (drv && drv->ops && drv->ops->get_freq) {
		r = drv->ops->get_freq(clk, index, &buf);
		if (freq) {
			*freq = buf;
		}
	}

	return r;
}

int k_clk_set_frequency(struct k_clk_device *clk, int index, uint64_t freq)
{
	const struct k_clk_driver *drv = k_clk_get_drv(clk);
	int r = -ENODEV;

	if (drv && drv->ops && drv->ops->set_freq) {
		r = drv->ops->set_freq(clk, index, freq);
	}

	return r;
}

int k_clk_get_clk_from_config(struct k_device *dev, int index, struct k_clk_device **clk, int *index_clk)
{
	const char *clock_name;
	struct k_device *tmp;
	uint32_t val;
	int r;

	r = k_device_read_conf_str(dev, "clocks", &clock_name, index * 2);
	if (r) {
		k_dev_err(dev, "clock name is not found, index:%d.\n", index);
		return -EINVAL;
	}
	r = k_device_read_conf_u32(dev, "clocks", &val, index * 2 + 1);
	if (r) {
		k_dev_err(dev, "clock number is not found, index:%d.\n", index);
		return -EINVAL;
	}

	r = k_bus_find_device(k_bus_get_root(), clock_name, &tmp);
	if (r == -EAGAIN) {
		return -EAGAIN;
	} else if (r) {
		k_dev_err(dev, "clock '%s' is not found.\n", clock_name);
		return -EINVAL;
	}

	if (clk) {
		*clk = k_clk_from_dev(tmp);
	}
	if (index_clk) {
		*index_clk = val;
	}

	return 0;
}
