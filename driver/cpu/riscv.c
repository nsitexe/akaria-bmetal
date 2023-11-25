/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/cpu.h>
#include <bmetal/arch.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/driver/intc.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

uintptr_t __section(".noinit") k_boot_proc;
uintptr_t k_boot_sp_idle;
uintptr_t k_boot_sp_intr;
int k_boot_done;

struct cpu_riscv_priv {
};
CHECK_PRIV_SIZE_CPU(struct cpu_riscv_priv);

static int cpu_riscv_add(struct k_device *dev)
{
	struct k_cpu_device *cpu = k_cpu_from_dev(dev);
	struct cpu_riscv_priv *priv = dev->priv;
	uint32_t hartid;
	int cpuid, r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	/* Logical and physical ID */
	r = k_device_read_conf_u32(dev, "hartid", &hartid, 0);
	if (r) {
		k_dev_err(dev, "config 'hartid' is not found.\n");
		return -EINVAL;
	}

	if (hartid == CONFIG_MAIN_CORE) {
		cpuid = 0;
	} else {
		cpuid = k_cpu_alloc_id();
	}
	if (cpuid >= CONFIG_NUM_CORES) {
		k_dev_err(dev, "add too many cpus, max:%d.\n", CONFIG_NUM_CORES);
		return -EINVAL;
	}

	cpu->id_cpu = cpuid;
	cpu->id_phys = hartid;

	/* Zic64b requests that cache line size must be 64 bytes */
	cpu->line_size_i = 64;
	cpu->line_size_d = 64;

	k_cpu_set(cpuid, cpu);

	return 0;
}

static int cpu_riscv_remove(struct k_device *dev)
{
	return -ENOTSUP;
}

static int cpu_riscv_clean_range(struct k_cpu_device *cpu, const void *start, size_t sz)
{
#ifdef CONFIG_CACHE
	int sz_line = k_cpu_cache_get_line_size_d(cpu);
	const char *p = start;

	if (sz_line <= 0) {
		k_dev_err(k_cpu_to_dev(cpu), "cache line size %d is invalid.\n", sz_line);
		return -EINVAL;
	}

	for (size_t i = 0; i < sz; i += sz_line) {
		__asm volatile ("cbo.clean %0" :: "A"(p[i]));
	}

	dwmb();
#endif /* CONFIG_CACHE */

	return 0;
}

static int cpu_riscv_inv_range(struct k_cpu_device *cpu, const void *start, size_t sz)
{
#ifdef CONFIG_CACHE
	int sz_line = k_cpu_cache_get_line_size_d(cpu);
	const char *p = start;

	if (sz_line <= 0) {
		k_dev_err(k_cpu_to_dev(cpu), "cache line size %d is invalid.\n", sz_line);
		return -EINVAL;
	}

	for (size_t i = 0; i < sz; i += sz_line) {
		__asm volatile ("cbo.inval %0" :: "A"(p[i]));
	}

	dwmb();
#endif /* CONFIG_CACHE */

	return 0;
}

static int cpu_riscv_flush_range(struct k_cpu_device *cpu, const void *start, size_t sz)
{
#ifdef CONFIG_CACHE
	int sz_line = k_cpu_cache_get_line_size_d(cpu);
	const char *p = start;

	if (sz_line <= 0) {
		k_dev_err(k_cpu_to_dev(cpu), "cache line size %d is invalid.\n", sz_line);
		return -EINVAL;
	}

	for (size_t i = 0; i < sz; i += sz_line) {
		__asm volatile ("cbo.flush %0" :: "A"(p[i]));
	}

	dwmb();
#endif /* CONFIG_CACHE */

	return 0;
}

static int cpu_riscv_wakeup(struct k_cpu_device *cpu)
{
	size_t pos_idle = (cpu->id_cpu + 1) * CONFIG_IDLE_STACK_SIZE;
	size_t pos_intr = (cpu->id_cpu + 1) * CONFIG_INTR_STACK_SIZE;

	k_boot_done = 0;
	k_boot_sp_idle = (uintptr_t)&k_stack_idle[pos_idle];
	k_boot_sp_intr = (uintptr_t)&k_stack_intr[pos_intr];

	dwmb();

	while (!k_boot_done) {
		k_boot_proc = cpu->id_phys;
		dmb();
	}

	k_boot_proc = -1;
	dwmb();

	return 0;
}

static int cpu_riscv_sleep(struct k_cpu_device *cpu)
{
	return 0;
}

#if !defined(CONFIG_INTC)
static int cpu_riscv_raise_ipi(struct k_cpu_device *cpu, struct k_cpu_device *dest, void *arg)
{
	k_dev_err(k_cpu_to_dev(cpu), "Not support intc.\n");
	return -ENOTSUP;
}
#else /* CONFIG_INTC */
static int cpu_riscv_raise_ipi(struct k_cpu_device *cpu, struct k_cpu_device *dest, void *arg)
{
	int r;

	r = k_intc_raise_ipi(cpu, dest, arg);
	if (r) {
		return r;
	}

	return 0;
}
#endif /* CONFIG_INTC */

const static struct k_device_driver_ops cpu_riscv_dev_ops = {
	.add = cpu_riscv_add,
	.remove = cpu_riscv_remove,
};

const static struct k_cpu_driver_ops cpu_riscv_cpu_ops = {
	.clean_range = cpu_riscv_clean_range,
	.inv_range = cpu_riscv_inv_range,
	.flush_range = cpu_riscv_flush_range,
	.wakeup = cpu_riscv_wakeup,
	.sleep = cpu_riscv_sleep,
	.raise_ipi = cpu_riscv_raise_ipi,
};

static struct k_cpu_driver cpu_riscv_drv = {
	.base = {
		.base = {
			.type_vendor = "none",
			.type_device = "cpu_riscv",
		},

		.ops = &cpu_riscv_dev_ops,
	},

	.ops = &cpu_riscv_cpu_ops,
};

static int cpu_riscv_init(void)
{
	k_cpu_add_driver(&cpu_riscv_drv);

	return 0;
}

define_init_func(cpu_riscv_init);
