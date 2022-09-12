/* SPDX-License-Identifier: Apache-2.0 */

#include <stdatomic.h>

#include <bmetal/drivers/cpu.h>
#include <bmetal/arch.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

atomic_uintptr_t __section(".noinit") __boot_proc;
atomic_uintptr_t __boot_sp;
atomic_int __boot_done;

struct cpu_riscv_priv {
};
CHECK_PRIV_SIZE_CPU(struct cpu_riscv_priv);

static int cpu_riscv_add(struct __device *dev)
{
	struct __cpu_device *cpu = __cpu_from_dev(dev);
	struct cpu_riscv_priv *priv = dev->priv;
	uint32_t hartid;
	int cpuid, r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	/* Logical and physical ID */
	r = __device_read_conf_u32(dev, "hartid", &hartid, 0);
	if (r) {
		__dev_err(dev, "config 'hartid' is not found.\n");
		return -EINVAL;
	}

	if (hartid == CONFIG_MAIN_CORE) {
		cpuid = 0;
	} else {
		cpuid = __cpu_alloc_id();
	}
	if (cpuid >= CONFIG_NUM_CORES) {
		__dev_err(dev, "add too many cpus, max:%d.\n", CONFIG_NUM_CORES);
		return -EINVAL;
	}

	cpu->id_cpu = cpuid;
	cpu->id_phys = hartid;

	/* Zic64b requests that cache line size must be 64 bytes */
	cpu->line_size_i = 64;
	cpu->line_size_d = 64;

	__cpu_set(cpuid, cpu);

	return 0;
}

static int cpu_riscv_remove(struct __device *dev)
{
	return -ENOTSUP;
}

static int cpu_riscv_clean_range(struct __cpu_device *cpu, const void *start, size_t sz)
{
#ifdef CONFIG_RISCV_CACHE
	int sz_line = __cpu_cache_get_line_size_d(cpu);
	const char *p = start;

	if (sz_line <= 0) {
		__dev_err(__cpu_to_dev(cpu), "cache line size %d is invalid.\n", sz_line);
		return -EINVAL;
	}

	for (size_t i = 0; i < sz; i += sz_line) {
		__asm volatile ("cbo.clean %0" :: "A"(p[i]));
	}
#endif /* CONFIG_RISCV_CACHE */

	return 0;
}

static int cpu_riscv_inv_range(struct __cpu_device *cpu, const void *start, size_t sz)
{
#ifdef CONFIG_RISCV_CACHE
	int sz_line = __cpu_cache_get_line_size_d(cpu);
	const char *p = start;

	if (sz_line <= 0) {
		__dev_err(__cpu_to_dev(cpu), "cache line size %d is invalid.\n", sz_line);
		return -EINVAL;
	}

	for (size_t i = 0; i < sz; i += sz_line) {
		__asm volatile ("cbo.inval %0" :: "A"(p[i]));
	}
#endif /* CONFIG_RISCV_CACHE */

	return 0;
}

static int cpu_riscv_flush_range(struct __cpu_device *cpu, const void *start, size_t sz)
{
#ifdef CONFIG_RISCV_CACHE
	int sz_line = __cpu_cache_get_line_size_d(cpu);
	const char *p = start;

	if (sz_line <= 0) {
		__dev_err(__cpu_to_dev(cpu), "cache line size %d is invalid.\n", sz_line);
		return -EINVAL;
	}

	for (size_t i = 0; i < sz; i += sz_line) {
		__asm volatile ("cbo.flush %0" :: "A"(p[i]));
	}
#endif /* CONFIG_RISCV_CACHE */

	return 0;
}

static int cpu_riscv_wakeup(struct __cpu_device *cpu)
{
	size_t sp_pos = (cpu->id_cpu + 1) * CONFIG_INTR_STACK_SIZE;

	__boot_done = 0;
	__boot_sp = (uintptr_t)&__stack_intr[sp_pos];

	cpu->running = 1;

	dwmb();

	while (!__boot_done) {
		__boot_proc = cpu->id_phys;
		dmb();
	}

	return 0;
}

static int cpu_riscv_sleep(struct __cpu_device *cpu)
{
	cpu->running = 0;

	dwmb();

	return 0;
}

#if !defined(CONFIG_INTC)
static int cpu_riscv_raise_ipi(struct __cpu_device *cpu, struct __cpu_device *dest, void *arg)
{
	__dev_err(__cpu_to_dev(cpu), "Not support intc.\n");
	return -ENOTSUP;
}
#else /* CONFIG_INTC */
static int cpu_riscv_raise_ipi(struct __cpu_device *cpu, struct __cpu_device *dest, void *arg)
{
	int r;

	r = __intc_raise_ipi(cpu, dest, arg);
	if (r) {
		return r;
	}

	return 0;
}
#endif /* CONFIG_INTC */

const static struct __device_driver_ops cpu_riscv_dev_ops = {
	.add = cpu_riscv_add,
	.remove = cpu_riscv_remove,
};

const static struct __cpu_driver_ops cpu_riscv_cpu_ops = {
	.clean_range = cpu_riscv_clean_range,
	.inv_range = cpu_riscv_inv_range,
	.flush_range = cpu_riscv_flush_range,
	.wakeup = cpu_riscv_wakeup,
	.sleep = cpu_riscv_sleep,
	.raise_ipi = cpu_riscv_raise_ipi,
};

static struct __cpu_driver cpu_riscv_drv = {
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
	__cpu_add_driver(&cpu_riscv_drv);

	return 0;
}

define_init_func(cpu_riscv_init);
