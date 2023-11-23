/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_IO_H_
#define BAREMETAL_CRT_IO_H_

#include <stdint.h>

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

#define K_IO_MAP_FAILED    ((void *)(-1))
#define K_IO_FROM_PTR(v)   ((uintptr_t)(v))
#define K_IO_TO_PTR(v)     ((void *)(v))

struct k_device;

struct k_io_map {
	uintptr_t addr;
	uintptr_t size;
};

static inline uint8_t k_io_read8(volatile void *ptr)
{
	return *(volatile uint8_t *)ptr;
}

static inline uint16_t k_io_read16(volatile void *ptr)
{
	return *(volatile uint16_t *)ptr;
}

static inline uint32_t k_io_read32(volatile void *ptr)
{
	return *(volatile uint32_t *)ptr;
}

static inline uint64_t k_io_read64(volatile void *ptr)
{
	return *(volatile uint64_t *)ptr;
}

static inline void k_io_write8(uint8_t dat, volatile void *ptr)
{
	*(volatile uint8_t *)ptr = dat;
}

static inline void k_io_write16(uint16_t dat, volatile void *ptr)
{
	*(volatile uint16_t *)ptr = dat;
}

static inline void k_io_write32(uint32_t dat, volatile void *ptr)
{
	*(volatile uint32_t *)ptr = dat;
}

static inline void k_io_write64(uint64_t dat, volatile void *ptr)
{
	*(volatile uint64_t *)ptr = dat;
}

int k_io_mmap_device(void *addr, struct k_device *dev);
int k_io_munmap_device(void *addr, struct k_device *dev);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_IO_H_ */
