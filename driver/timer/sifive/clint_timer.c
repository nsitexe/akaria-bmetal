/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/drivers/timer.h>
#include <bmetal/arch.h>
#include <bmetal/clock.h>
#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/time.h>

/* Start address is 0x4000 of clint */
#define REG_MTIMECMP(hart)     (0x0000 + (hart) * 8)
#define REG_MTIMECMPL(hart)    (0x0000 + (hart) * 8)
#define REG_MTIMECMPH(hart)    (0x0004 + (hart) * 8)
#define REG_MTIME              0x7ff8
#define REG_MTIMEL             0x7ff8
#define REG_MTIMEH             0x7ffc

struct timer_clint_priv {
	struct k_timer_device *timer;
	struct k_clk_device *clk_in;
	int index_clk_in;
	uint64_t freq_in;
};
CHECK_PRIV_SIZE_TIMER(struct timer_clint_priv);

static int timer_clint_get_freq(struct k_timer_device *tm, int index, uint64_t *freq)
{
	struct __device *dev = k_timer_to_dev(tm);
	struct timer_clint_priv *priv = dev->priv;

	if (priv->freq_in == 0) {
		__dev_err(dev, "input clock frequency is 0.\n");
		return -EINVAL;
	}

	if (freq) {
		*freq = priv->freq_in;
	}

	return 0;
}

static int timer_clint_get_raw(struct k_timer_device *tm, int index, uint64_t *count)
{
	struct __device *dev = k_timer_to_dev(tm);
	uint64_t v;

#ifdef CONFIG_64BIT
	v = __device_read64(dev, REG_MTIME);
#else
	uint32_t vh1, vh2, vl;

	do {
		vh1 = __device_read32(dev, REG_MTIMEH);
		vl = __device_read32(dev, REG_MTIMEL);
		vh2 = __device_read32(dev, REG_MTIMEH);
	} while (vh1 != vh2);

	v = ((uint64_t)vh1 << 32) | vl;
#endif /* CONFIG_64BIT */

	if (count) {
		*count = v;
	}

	return 0;
}

static int timer_clint_get_timecmp(struct k_timer_device *tm, int index, uint64_t *count)
{
	struct __device *dev = k_timer_to_dev(tm);
	uint64_t v;

#ifdef CONFIG_64BIT
	v = __device_read64(dev, REG_MTIMECMP(index));
#else
	uint32_t vh, vl;

	vh = __device_read32(dev, REG_MTIMECMPH(index));
	vl = __device_read32(dev, REG_MTIMECMPL(index));
	v = ((uint64_t)vh << 32) | vl;
#endif /* CONFIG_64BIT */

	if (count) {
		*count = v;
	}

	return 0;
}

static int timer_clint_set_timecmp(struct k_timer_device *tm, int index, uint64_t count)
{
	struct __device *dev = k_timer_to_dev(tm);

#ifdef CONFIG_64BIT
	__device_write64(dev, count, REG_MTIMECMP(index));
#else
	__device_write32(dev, count >> 32, REG_MTIMECMPH(index));
	__device_write32(dev, count, REG_MTIMECMPL(index));
#endif /* CONFIG_64BIT */

	return 0;
}

static int timer_clint_get_trigger(struct k_timer_device *tm, int index, struct timespec64 *tsp)
{
	uint64_t cnt, freq;
	int r;

	r = timer_clint_get_freq(tm, index, &freq);
	if (r) {
		return r;
	}

	r = timer_clint_get_timecmp(tm, index, &cnt);
	if (r) {
		return r;
	}

	r = k_clock_raw_to_timespec(cnt, freq, tsp);
	if (r) {
		return r;
	}

	return 0;
}

static int timer_clint_set_trigger(struct k_timer_device *tm, int index, const struct timespec64 *tsp)
{
	uint64_t cnt, freq;
	int r;

	r = timer_clint_get_freq(tm, index, &freq);
	if (r) {
		return r;
	}

	r = k_clock_timespec_to_raw(tsp, freq, &cnt);
	if (r) {
		return r;
	}

	r = timer_clint_set_timecmp(tm, index, cnt);
	if (r) {
		return r;
	}

	return 0;
}

static int timer_clint_intr(int event, struct __event_handler *hnd)
{
	int r;

	r = k_clock_on_tick();
	if (r) {
		return EVENT_NOT_HANDLED;
	}

	return EVENT_HANDLED;
}

static int timer_clint_add(struct __device *dev)
{
	struct timer_clint_priv *priv = dev->priv;
	struct k_timer_device *timer = k_timer_from_dev(dev);
	struct k_intc_device *intc_parent;
	int len, num_irq, r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->timer = timer;

	/* Clock */
	r = k_clk_get_clk_from_config(dev, 0, &priv->clk_in, &priv->index_clk_in);
	if (r) {
		return r;
	}

	r = k_clk_get_frequency(priv->clk_in, priv->index_clk_in, &priv->freq_in);
	if (r) {
		__dev_err(dev, "clock freq is unknown.\n");
		return r;
	}

	r = k_clk_enable(priv->clk_in, priv->index_clk_in);
	if (r) {
		return r;
	}

	/* Register */
	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	/* Interrupt */
	r = k_intc_get_conf_length(dev, &len);
	if (r) {
		return r;
	}

	for (int i = 0; i < len; i++) {
		struct __event_handler *hnd;

		r = k_intc_get_intc_from_config(dev, i, &intc_parent, &num_irq);
		if (r) {
			return r;
		}

		r = __event_alloc_handler(&hnd);
		if (r) {
			return r;
		}

		hnd->func = timer_clint_intr;
		hnd->priv = priv;

		r = k_intc_add_handler(intc_parent, num_irq, hnd);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int timer_clint_remove(struct __device *dev)
{
	return 0;
}

const static struct __device_driver_ops timer_clint_dev_ops = {
	.add = timer_clint_add,
	.remove = timer_clint_remove,
	.mmap = __device_driver_mmap,
};

const static struct k_timer_driver_ops timer_clint_timer_ops = {
	.get_freq = timer_clint_get_freq,
	.get_raw = timer_clint_get_raw,
	.get_trigger = timer_clint_get_trigger,
	.set_trigger = timer_clint_set_trigger,
};

static struct k_timer_driver timer_clint_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "clint0_timer",
		},

		.ops = &timer_clint_dev_ops,
	},

	.ops = &timer_clint_timer_ops,
};

static int timer_clint_init(void)
{
	k_timer_add_driver(&timer_clint_drv);

	return 0;
}
define_init_func(timer_clint_init);
