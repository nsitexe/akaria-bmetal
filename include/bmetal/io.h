/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_IO_H_
#define BAREMETAL_CRT_IO_H_

#include <stdint.h>

#include <bmetal/bmetal.h>

#define __IO_MAP_FAILED    ((void *)(-1))
#define __IO_FROM_PTR(v)   ((uintptr_t)(v))
#define __IO_TO_PTR(v)     ((void *)(v))

struct __device;

struct __io_map {
	uintptr_t addr;
	uintptr_t size;
};

static inline uint8_t __io_read8(volatile void *ptr)
{
	return *(volatile uint8_t *)ptr;
}

static inline uint16_t __io_read16(volatile void *ptr)
{
	return *(volatile uint16_t *)ptr;
}

static inline uint32_t __io_read32(volatile void *ptr)
{
	return *(volatile uint32_t *)ptr;
}

static inline void __io_write8(uint8_t dat, volatile void *ptr)
{
	*(volatile uint8_t *)ptr = dat;
}

static inline void __io_write16(uint16_t dat, volatile void *ptr)
{
	*(volatile uint16_t *)ptr = dat;
}

static inline void __io_write32(uint32_t dat, volatile void *ptr)
{
	*(volatile uint32_t *)ptr = dat;
}

int __io_mmap_device(void *addr, struct __device *dev);
int __io_munmap_device(void *addr, struct __device *dev);

#endif /* BAREMETAL_CRT_IO_H_ */
