/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdatomic.h>

#include <bmetal/drivers/cpu.h>
#include <bmetal/arch.h>
#include <bmetal/init.h>
#include <bmetal/inttypes.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/drivers/intc.h>

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

	__cpu_set(cpuid, cpu);

	return 0;
}

static int cpu_riscv_remove(struct __device *dev)
{
	return -ENOTSUP;
}

static int cpu_riscv_wakeup(struct __cpu_device *cpu)
{
	size_t sp_pos = (cpu->id_cpu + 1) * CONFIG_INTR_STACK_SIZE;

	__boot_done = 0;
	__boot_sp = (uintptr_t)&__stack_intr[sp_pos];
	dwmb();

	while (!__boot_done) {
		__boot_proc = cpu->id_phys;
		dmb();
	}

	cpu->running = 1;

	dwmb();

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
