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
#else /* CONFIG_XIP */
#  define TEXT_AREA       RAM
#  define DATA_AREA       RAM
#  define DATA_AREA_AT    RAM
#endif /* CONFIG_XIP */

OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY
{
	ROM (rx)  : ORIGIN = CONFIG_ROM_BASE, LENGTH = CONFIG_ROM_SIZE
	RAM (rwx) : ORIGIN = CONFIG_RAM_BASE, LENGTH = CONFIG_RAM_SIZE
}

SECTIONS
{
	PROVIDE(_text = .);
	PROVIDE(_stext = .);

	/* Special text section for reset vector */
	.reset : {
		. = ALIGN(4);
		KEEP(*(.reset .reset.*))
	} > TEXT_AREA

	.text : {
		. = ALIGN(4);
		*(.text .text.*)
	} > TEXT_AREA

	.fini : {
		. = ALIGN(4);
		KEEP(*(SORT_NONE(.fini)))
	} > TEXT_AREA

	PROVIDE(_etext = .);

	.rodata : {
		. = ALIGN(4);
		*(.rodata .rodata.*)
	} > TEXT_AREA

	.data : {
		. = ALIGN(4);
		__data_start = .;
		*(.data .data.*)
	} > DATA_AREA AT > DATA_AREA_AT

	.sdata : {
		. = ALIGN(4);
		__global_pointer$ = . + 0x800;
		*(.srodata.*)
		*(.sdata .sdata.*)
		__data_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__data_load = LOADADDR(.data);
	PROVIDE(_edata = .);

	.sbss (NOLOAD) : {
		. = ALIGN(4);
		__bss_start = .;
		*(.sbss .sbss.*)
		*(.scommon)
	} > DATA_AREA

	.bss (NOLOAD) : {
		. = ALIGN(4);
		*(.bss .bss.*)
		*(COMMON)
		__bss_end = .;
	} > DATA_AREA

	__bss_load = LOADADDR(.sbss);
	PROVIDE(_end = .);
}
