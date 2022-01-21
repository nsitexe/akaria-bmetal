/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/generated/autoconf.h>

#ifndef CONFIG_ROM_BASE
#  error Not defined base address of ROM (CONFIG_ROM_BASE).
#endif
#ifndef CONFIG_ROM_SIZE
#  error Not defined size of ROM (CONFIG_ROM_SIZE).
#endif
#ifndef CONFIG_RAM_BASE
#  error Not defined base address of RAM (CONFIG_RAM_BASE).
#endif
#ifndef CONFIG_RAM_SIZE
#  error Not defined size of RAM (CONFIG_RAM_SIZE).
#endif

#ifdef CONFIG_XIP
#  define TEXT_AREA       ROM
#  define DATA_AREA       RAM
#  define DATA_AREA_AT    ROM
#  define COMM_AREA       SHM
#else /* CONFIG_XIP */
#  define TEXT_AREA       RAM
#  define DATA_AREA       RAM
#  define DATA_AREA_AT    RAM
#  define COMM_AREA       SHM
#endif /* CONFIG_XIP */

OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY
{
	ROM (rx)  : ORIGIN = CONFIG_ROM_BASE, LENGTH = CONFIG_ROM_SIZE
	RAM (rwx) : ORIGIN = CONFIG_RAM_BASE, LENGTH = CONFIG_RAM_SIZE
	SHM (rwx) : ORIGIN = CONFIG_SHM_BASE, LENGTH = CONFIG_SHM_SIZE
}

SECTIONS
{
	PROVIDE(_text = .);
	PROVIDE(_stext = .);

	/* Special text section for reset vector */
	.reset : {
		. = ALIGN(8);
		KEEP(*(.reset .reset.*))
	} > TEXT_AREA

	.init : {
		. = ALIGN(8);
		KEEP(*(SORT_NONE(.init)))
	} > TEXT_AREA

	.text : {
		. = ALIGN(8);
		*(.text .text.*)
	} > TEXT_AREA

	.fini : {
		. = ALIGN(8);
		KEEP(*(SORT_NONE(.fini)))
	} > TEXT_AREA

	PROVIDE(_etext = .);

	.comm : {
		. = ALIGN(8);
		KEEP(*(.comm_header .comm_header.*))
	} > TEXT_AREA

	.rodata : {
		. = ALIGN(8);
		*(.rodata .rodata.*)
	} > TEXT_AREA

	.initdata : {
		. = ALIGN(8);
		__initcall_start = .;
		KEEP(*(.initdata .initdata.*))
		__initcall_end = .;
	} > TEXT_AREA

	.data : {
		. = ALIGN(8);
		__data_start = .;
		*(.data .data.*)
		__data_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__data_load = LOADADDR(.data);

	.sdata : {
		. = ALIGN(8);
		__sdata_start = .;
		__global_pointer$ = . + 0x800;
		*(.srodata.*)
		*(.sdata .sdata.*)
		__sdata_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__sdata_load = LOADADDR(.sdata);
	PROVIDE(_edata = .);

	.sbss (NOLOAD) : {
		. = ALIGN(8);
		__sbss_start = .;
		*(.sbss .sbss.*)
		*(.scommon)
		__sbss_end = .;
	} > DATA_AREA

	.bss (NOLOAD) : {
		. = ALIGN(8);
		__bss_start = .;
		*(.bss .bss.*)
		*(COMMON)
		__bss_end = .;
	} > DATA_AREA

	.noinit (NOLOAD) : {
		. = ALIGN(8);
		__noinit_start = .;
		*(.noinit .noinit.*)
		__noinit_end = .;
	} > DATA_AREA

	.comm_area (NOLOAD) : {
		. = ALIGN(8);
		PROVIDE(__comm_area = .);
		. = . + CONFIG_COMM_AREA_SIZE;
	} > COMM_AREA

	PROVIDE(_end = .);
}
