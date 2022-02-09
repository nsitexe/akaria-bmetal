/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <inttypes.h>
#include <stdatomic.h>

#include <bmetal/drivers/cpu.h>
#include <bmetal/arch.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>

atomic_uintptr_t __section(".noinit") __boot_proc;
atomic_uintptr_t __boot_sp;
atomic_uintptr_t __boot_tp;
atomic_int __boot_done;

static inline int get_hartid(void)
{
	int tmp;

	__asm volatile ("csrr %0, mhartid" : "=r"(tmp));

	return tmp;
}

static int cpu_riscv_add(struct __device *dev)
{
	struct __cpu_device *cpu = __cpu_from_dev(dev);
	uint32_t hartid;
	int cpuid, r;

	r = __device_read_conf_u32(dev, "hartid", &hartid);
	if (r) {
		printk("riscv_add: config 'hartid' is not found.\n");
		return -EINVAL;
	}

	if (hartid == CONFIG_MAIN_CORE) {
		cpuid = 0;
	} else {
		cpuid = __cpu_alloc_id();
	}
	if (cpuid >= CONFIG_NUM_CORES) {
		printk("riscv_add: add too many cpus, max:%d.\n", CONFIG_NUM_CORES);
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
	__boot_tp = (uintptr_t)__get_raw_thread(cpu->id_cpu);
	dwmb();

	__boot_proc = cpu->id_phys;
	dmb();

	while (!__boot_done) {
		drmb();
	}

	return 0;
}

const static struct __device_driver_ops cpu_riscv_dev_ops = {
	.add = cpu_riscv_add,
	.remove = cpu_riscv_remove,
};

static struct __cpu_driver cpu_riscv_drv = {
	.base = {
		.base = {
			.type_vendor = "generic",
			.type_device = "cpu_riscv",
		},

		.ops = &cpu_riscv_dev_ops,
	},

	.wakeup = cpu_riscv_wakeup,
};

static int cpu_riscv_init(void)
{
	__cpu_add_driver(&cpu_riscv_drv);

	return 0;
}

define_init_func(cpu_riscv_init);
