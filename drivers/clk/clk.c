/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/clk.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>

int __clk_add_device(struct __clk_device *clk, struct __bus *parent)
{
	int r;

	r = __device_add(__clk_to_dev(clk), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	return 0;
}

int __clk_remove_device(struct __clk_device *clk)
{
	return __device_remove(__clk_to_dev(clk));
}

int __clk_enable(struct __clk_device *clk, int index)
{
	const struct __clk_driver *drv = __clk_get_drv(clk);
	int r = -ENODEV;

	if (drv && drv->ops && drv->ops->enable) {
		r = drv->ops->enable(clk, index);
	}

	return r;
}

int __clk_disable(struct __clk_device *clk, int index)
{
	const struct __clk_driver *drv = __clk_get_drv(clk);
	int r = -ENODEV;

	if (drv && drv->ops && drv->ops->disable) {
		r = drv->ops->disable(clk, index);
	}

	return r;
}

int __clk_get_frequency(struct __clk_device *clk, int index, uint64_t *freq)
{
	const struct __clk_driver *drv = __clk_get_drv(clk);
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

int __clk_set_frequency(struct __clk_device *clk, int index, uint64_t freq)
{
	const struct __clk_driver *drv = __clk_get_drv(clk);
	int r = -ENODEV;

	if (drv && drv->ops && drv->ops->set_freq) {
		r = drv->ops->set_freq(clk, index, freq);
	}

	return r;
}

int __clk_get_clk_from_config(struct __device *dev, int index, struct __clk_device **clk)
{
	const char *clock_name;
	struct __device *tmp;
	uint32_t val;
	int r;

	r = __device_read_conf_str(dev, "clocks", &clock_name, index * 2);
	if (r) {
		__dev_err(dev, "clock name is not found, index:%d.\n", index);
		return -EINVAL;
	}
	r = __device_read_conf_u32(dev, "clocks", &val, index * 2 + 1);
	if (r) {
		__dev_err(dev, "clock number is not found, index:%d.\n", index);
		return -EINVAL;
	}

	r = __bus_find_device(__bus_get_root(), clock_name, &tmp);
	if (r == -EAGAIN) {
		return -EAGAIN;
	} else if (r) {
		__dev_err(dev, "clock '%s' is not found.\n", clock_name);
		return -EINVAL;
	}

	if (clk) {
		*clk = __clk_from_dev(tmp);
	}

	return 0;
}
