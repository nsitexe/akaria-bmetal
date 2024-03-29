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
ENTRY(k_reset)

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
	/* Special text section for reset vector */
	.reset : {
		. = ALIGN(4);
		PROVIDE(__executable_start = .);
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

	.comm : {
		KEEP(*(.comm_header .comm_header.*))
	} > TEXT_AREA AT > TEXT_AREA_AT

	.initdata : {
		. = ALIGN(8);
		k_initcall_start = .;
		KEEP(*(.initdata .initdata.*))
		k_initcall_end = .;
	} > TEXT_AREA AT > TEXT_AREA_AT

	.auxdata : {
		*(.auxdata .auxdata.*)
		PROVIDE(__ehdr_start = .);
		. += CONFIG_AUX_AREA_SIZE;
	} > TEXT_AREA AT > TEXT_AREA_AT

	.rodata : {
		*(.rodata .rodata.*)
	} > TEXT_AREA AT > TEXT_AREA_AT

	.tdata : {
		k_tdata_start = .;
		*(.tdata .tdata.* .gnu.linkonce.td.*)
		k_tdata_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	k_tdata_load = LOADADDR(.tdata);

	.tbss (NOLOAD) : {
		*(.tbss .tbss.* .gnu.linkonce.tb.*)
		*(.tcommon)
	} > DATA_AREA

	.preinit_array : {
		__preinit_array_start = .;
		KEEP(*(.preinit_array))
		__preinit_array_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__preinit_array_load = LOADADDR(.preinit_array);

	.init_array : {
		__init_array_start = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)
			SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .ctors))
		__init_array_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__init_array_load = LOADADDR(.init_array);

	.fini_array : {
		__fini_array_start = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)
			SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		__fini_array_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	__fini_array_load = LOADADDR(.fini_array);

	.data : {
		k_data_start = .;
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
		k_data_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	k_data_load = LOADADDR(.data);

	.sdata : {
		k_sdata_start = .;
		__global_pointer$ = . + 0x800;
		*(.srodata .srodata.*)
		*(.sdata .sdata.* .gnu.linkonce.s.*)
		k_sdata_end = .;
	} > DATA_AREA AT > DATA_AREA_AT

	k_sdata_load = LOADADDR(.sdata);

	.sbss (NOLOAD) : {
		k_sbss_start = .;
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
		k_sbss_end = .;
	} > DATA_AREA

	.bss (NOLOAD) : {
		k_bss_start = .;
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		/* for glibc */
		*(__libc_freeres_ptrs)
		k_bss_end = .;
	} > DATA_AREA

	.noinit (NOLOAD) : {
		*(.noinit .noinit.*)
	} > DATA_AREA

	.heap (NOLOAD) : {
		*(.heap .heap.*)
	} > DATA_AREA

	.comm_area (NOLOAD) : {
		. = ALIGN(8);
		PROVIDE(k_comm_area = .);
		. = . + CONFIG_COMM_AREA_SIZE;
	} > COMM_AREA

	/* for newlib */
	PROVIDE(_edata = .);
	PROVIDE(__bss_start = .);

	PROVIDE(_end = .);
}
