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
	} > ROM

	.text : {
		*(.text .text.*)
	} > ROM
	.fini : {
		KEEP(*(SORT_NONE(.fini)))
	} > ROM

	PROVIDE(_etext = .);

	.rodata : {
		*(.rodata .rodata.*)
	} > ROM

	.data : {
		*(.data .data.*)
	} > RAM AT > ROM

	PROVIDE(_edata = .);

	.sdata : {
		__global_pointer$ = . + 0x800;
		*(.srodata.*)
		*(.sdata .sdata.*)
	} > RAM AT > ROM

	.sbss (NOLOAD) : {
		*(.sbss .sbss.*)
		*(.scommon)
	} > RAM AT > RAM

	.bss (NOLOAD) : {
		*(.bss .bss.*)
		*(COMMON)
	} > RAM AT > RAM

	PROVIDE(_end = .);
}
