/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_ELF_H_
#define BAREMETAL_CRT_SYS_ELF_H_

#include <stdint.h>

#include <bmetal/bmetal.h>

/* ELF32/64 */
typedef unsigned char Elf_Byte;

/* ELF32 */
typedef uint32_t    Elf32_Addr;
typedef uint32_t    Elf32_Off;
typedef uint16_t    Elf32_Section;
typedef uint16_t    Elf32_Versym;
typedef uint16_t    Elf32_Half;
typedef int32_t     Elf32_Sword;
typedef uint32_t    Elf32_Word;

/* ELF64 */
typedef uint64_t    Elf64_Addr;
typedef uint64_t    Elf64_Off;
typedef uint16_t    Elf64_Section;
typedef uint16_t    Elf64_Versym;
typedef uint16_t    Elf64_Half;
typedef int32_t     Elf64_Sword;
typedef uint32_t    Elf64_Word;
typedef int64_t     Elf64_Sxword;
typedef uint64_t    Elf64_Xword;

/* ELF header */
#define EI_NIDENT    16

typedef struct elf32_ehdr {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half    e_type;
	Elf32_Half    e_machine;
	Elf32_Word    e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	Elf32_Word    e_flags;
	Elf32_Half    e_ehsize;
	Elf32_Half    e_phentsize;
	Elf32_Half    e_phnum;
	Elf32_Half    e_shentsize;
	Elf32_Half    e_shnum;
	Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct elf64_ehdr {
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half    e_type;
	Elf64_Half    e_machine;
	Elf64_Word    e_version;
	Elf64_Addr    e_entry;
	Elf64_Off     e_phoff;
	Elf64_Off     e_shoff;
	Elf64_Word    e_flags;
	Elf64_Half    e_ehsize;
	Elf64_Half    e_phentsize;
	Elf64_Half    e_phnum;
	Elf64_Half    e_shentsize;
	Elf64_Half    e_shnum;
	Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

typedef struct elf32_phdr {
	Elf32_Word     p_type;
	Elf32_Off      p_offset;
	Elf32_Addr     p_vaddr;
	Elf32_Addr     p_paddr;
	Elf32_Word     p_filesz;
	Elf32_Word     p_memsz;
	Elf32_Word     p_flags;
	Elf32_Word     p_align;
} Elf32_Phdr;

typedef struct elf64_phdr {
	Elf64_Word     p_type;
	Elf64_Word     p_flags;
	Elf64_Off      p_offset;
	Elf64_Addr     p_vaddr;
	Elf64_Addr     p_paddr;
	Elf64_Xword    p_filesz;
	Elf64_Xword    p_memsz;
	Elf64_Xword    p_align;
} Elf64_Phdr;

typedef struct elf32_shdr {
	Elf32_Word     sh_name;
	Elf32_Word     sh_type;
	Elf32_Word     sh_flags;
	Elf32_Addr     sh_addr;
	Elf32_Off      sh_offset;
	Elf32_Word     sh_size;
	Elf32_Word     sh_link;
	Elf32_Word     sh_info;
	Elf32_Word     sh_addralign;
	Elf32_Word     sh_entsize;
} Elf32_Shdr;

typedef struct elf64_shdr {
	Elf64_Word     sh_name;
	Elf64_Word     sh_type;
	Elf64_Xword    sh_flags;
	Elf64_Addr     sh_addr;
	Elf64_Off      sh_offset;
	Elf64_Xword    sh_size;
	Elf64_Word     sh_link;
	Elf64_Word     sh_info;
	Elf64_Xword    sh_addralign;
	Elf64_Xword    sh_entsize;
} Elf64_Shdr;

#define AT_NULL      0
#define AT_IGNORE    1
#define AT_EXECFD    2
#define AT_PHDR      3
#define AT_PHENT     4
#define AT_PHNUM     5
#define AT_RANDOM    25

#endif /* BAREMETAL_CRT_SYS_ELF_H_ */
