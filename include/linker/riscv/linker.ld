/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/generated/autoconf.h>

#ifdef CONFIG_XIP
#ifndef CONFIG_ROM_BASE
#  error Not defined base address of ROM (CONFIG_ROM_BASE).
#endif
#ifndef CONFIG_ROM_SIZE
#  error Not defined size of ROM (CONFIG_ROM_SIZE).
#endif
#endif /* CONFIG_XIP */

#ifndef CONFIG_RAM_BASE
#  error Not defined base address of RAM (CONFIG_RAM_BASE).
#endif
#ifndef CONFIG_RAM_SIZE
#  error Not defined size of RAM (CONFIG_RAM_SIZE).
#endif
#ifndef CONFIG_SHM_BASE
#  error Not defined base address of SHARED RAM (CONFIG_SHM_BASE).
#endif
#ifndef CONFIG_SHM_SIZE
#  error Not defined size of SHARED RAM (CONFIG_SHM_SIZE).
#endif

#ifdef CONFIG_MIRROR_RAM
#ifndef CONFIG_MRR_BASE
#  error Not defined base address of MIRROR (CONFIG_MRR_BASE).
#endif
#ifndef CONFIG_MRR_SIZE
#  error Not defined size of MIRROR (CONFIG_MRR_SIZE).
#endif
#endif /* CONFIG_MIRROR_RAM */

#if defined(CONFIG_XIP) && defined(CONFIG_MIRROR_RAM)
#  define TEXT_AREA       ROM
#  define TEXT_AREA_AT    MRR
#  define DATA_AREA       RAM
#  define DATA_AREA_AT    MRR
#  define COMM_AREA       SHM
#elif defined(CONFIG_XIP) && !defined(CONFIG_MIRROR_RAM)
#  define TEXT_AREA       ROM
#  define TEXT_AREA_AT    ROM
#  define DATA_AREA       RAM
#  define DATA_AREA_AT    ROM
#  define COMM_AREA       SHM
#else /* !CONFIG_XIP */
#  define TEXT_AREA       RAM
#  define TEXT_AREA_AT    RAM
#  define DATA_AREA       RAM
#  define DATA_AREA_AT    RAM
#  define COMM_AREA       SHM
#endif /* CONFIG_XIP */

OUTPUT_ARCH(riscv)
ENTRY(__reset)

MEMORY
{
#ifdef CONFIG_XIP
	ROM (rx)  : ORIGIN = CONFIG_ROM_BASE, LENGTH = CONFIG_ROM_SIZE
#endif /* CONFIG_XIP */
	RAM (rwx) : ORIGIN = CONFIG_RAM_BASE, LENGTH = CONFIG_RAM_SIZE
	SHM (rwx) : ORIGIN = CONFIG_SHM_BASE, LENGTH = CONFIG_SHM_SIZE
#ifdef CONFIG_MIRROR_RAM
	MRR (rx)  : ORIGIN = CONFIG_MRR_BASE, LENGTH = CONFIG_MRR_SIZE
#endif /* CONFIG_MIRROR_RAM */
}

SECTIONS
{
	PROVIDE(_text = .);
	PROVIDE(_stext = .);

	/* Special text section for reset vector */
	.reset : {
		. = ALIGN(4);
		KEEP(*(.reset .reset.*))
	} > TEXT_AREA AT > TEXT_AREA_AT

	.init : {
		KEEP(*(SORT_NONE(.init)))
	} > TEXT_AREA AT > TEXT_AREA_AT

	.text : {
		. = ALIGN(4);
		*(.text .text.*)
		/* for glibc */
		*(__libc_freeres_fn)
	} > TEXT_AREA AT > TEXT_AREA_AT

	.fini : {
		KEEP(*(SORT_NONE(.fini)))
	} > TEXT_AREA AT > TEXT_AREA_AT

	PROVIDE(_etext = .);

	.comm : {
		KEEP(*(.comm_header .comm_header.*))
	} > TEXT_AREA AT > TEXT_AREA_AT

	.initdata : {
		. = ALIGN(8);
		__initcall_start = .;
		KEEP(*(.initdata .initdata.*))
		__initcall_end = .;
	} > TEXT_AREA AT > TEXT_AREA_AT

	.auxdata : {
		*(.auxdata .auxdata.*)
		PROVIDE(__ehdr_start = .);
		. += CONFIG_AUX_AREA_SIZE;
	} > TEXT_AREA AT > TEXT_AREA_AT

	.rodata : {
		*(.rodata .rodata.*)
	} > TEXT_AREA AT > TEXT_AREA_AT

	.preinit_array : {
		__preinit_array_start = .;
		KEEP(*(.preinit_array))
		__preinit_array_end = .;
	} > TEXT_AREA AT > TEXT_AREA_AT

	.init_array : {
		__init_array_start = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)
			SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .ctors))
		__init_array_end = .;
	} > TEXT_AREA AT > TEXT_AREA_AT

	.fini_array : {
		__fini_array_start = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		__fini_array_end = .;
	} > TEXT_AREA AT > TEXT_AREA_AT

	.tdata : {
		__tdata_start = .;
		*(.tdata .tdata.* .gnu.linkonce.td.*)
		__tdata_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__tdata_load = LOADADDR(.tdata);

	.tbss (NOLOAD) : {
		*(.tbss .tbss.* .gnu.linkonce.tb.*)
		*(.tcommon)
	} > DATA_AREA

	.data : {
		__data_start = .;
		*(.data .data.* .gnu.linkonce.d.*)
		*(.eh_frame .eh_frame.*)
		*(.eh_frame_hdr .eh_frame_hdr.* .eh_frame_entry .eh_frame_entry.*)
		*(.gcc_except_table .gcc_except_table.*)
		*(.got .got.plt)

		/* for glibc */
		__start___libc_subfreeres = .;
		*(__libc_subfreeres)
		__stop___libc_subfreeres = .;
		__start___libc_IO_vtables = .;
		*(__libc_IO_vtables)
		__stop___libc_IO_vtables = .;
		__start___libc_atexit = .;
		*(__libc_atexit)
		__stop___libc_atexit = .;
		__data_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__data_load = LOADADDR(.data);

	.sdata : {
		__sdata_start = .;
		__global_pointer$ = . + 0x800;
		*(.srodata .srodata.*)
		*(.sdata .sdata.* .gnu.linkonce.s.*)
		__sdata_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__sdata_load = LOADADDR(.sdata);
	PROVIDE(_edata = .);

	.sbss (NOLOAD) : {
		__sbss_start = .;
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
		__sbss_end = .;
	} > DATA_AREA

	.bss (NOLOAD) : {
		__bss_start = .;
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		/* for glibc */
		*(__libc_freeres_ptrs)
		__bss_end = .;
	} > DATA_AREA

	.noinit (NOLOAD) : {
		*(.noinit .noinit.*)
	} > DATA_AREA

	.comm_area (NOLOAD) : {
		. = ALIGN(8);
		PROVIDE(__comm_area = .);
		. = . + CONFIG_COMM_AREA_SIZE;
	} > COMM_AREA

	PROVIDE(_end = .);
}
