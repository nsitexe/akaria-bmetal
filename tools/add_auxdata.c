/* SPDX-License-Identifier: Apache-2.0 */

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#define log_err     printf
#define log_info    printf
#define log_dbg     no_printf

struct __aux_data {
	uint32_t valid;
	uint64_t phent;
	uint64_t phnum;
	uint64_t phdr_size;
};

struct prg_header {
	// Program header
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;

	// Memory chunk (size: p_filesz)
	unsigned int index;
	const uint8_t *src;
};

struct sec_header {
	// Section header
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint64_t sh_addralign;
	uint64_t sh_entsize;
};

struct elf_info {
	int magic;

	size_t elf_size;
	uint8_t *buf;
	const uint8_t *p_hdr_ent;
	const uint8_t *s_hdr_ent;
	// Section header strings
	const char *shstr_ent;
	size_t shstr_size;

	// ELF header
	uint32_t elf_class;
	uint32_t e_type;
	uint32_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint32_t e_ehsize;
	uint32_t e_phentsize;
	uint32_t e_phnum;
	uint32_t e_shentsize;
	uint32_t e_shnum;
	uint32_t e_shstrndx;

	// Program headers (len: e_phnum)
	struct prg_header *phs;
	// Section headers (len: e_shnum)
	struct sec_header *shs;
};

int no_printf(const char *fmt, ...)
{
	return 0;
}

static int elf_init(struct elf_info *elf)
{
	elf->buf = NULL;
	elf->phs = NULL;
	elf->shs = NULL;

	return 0;
}

static int elf_fini(struct elf_info *elf)
{
	if (elf->shs != NULL) {
		free(elf->shs);
		elf->shs = NULL;
	}

	if (elf->phs != NULL) {
		free(elf->phs);
		elf->phs = NULL;
	}

	elf->buf = NULL;

	return 0;
}

static int elf_get_class(const uint8_t *e_ident, int *parch)
{
	int arch;

	// ELF magic number
	if ((e_ident[0] != ELFMAG0)
	    || (e_ident[1] != ELFMAG1)
	    || (e_ident[2] != ELFMAG2)
	    || (e_ident[3] != ELFMAG3)) {
		log_err("[elf] invalid magic number\n");
		return -EINVAL;
	}
	log_dbg("  ELF ident      : %02x %02x %02x %02x %02x\n",
		e_ident[0], e_ident[1],
		e_ident[2], e_ident[3],
		e_ident[4]);

	// architecture class
	switch (e_ident[EI_CLASS]) {
	case ELFCLASS32:
		log_dbg("  ELF class      : 32\n");
		arch = 32;
		break;
	case ELFCLASS64:
		log_dbg("  ELF class      : 64\n");
		arch = 64;
		break;
	default:
		log_err("[elf] unknown architecture class\n");
		return -EINVAL;
	}
	if (parch != NULL) {
		*parch = arch;
	}

	return 0;
}

static int elf_is_valid_header(struct elf_info *elf)
{
	if (elf->e_type != ET_EXEC) {
		log_err("[elf] support only executable type\n");
		return -EINVAL;
	}

	if (elf->e_machine != EM_RISCV) {
		log_err("[elf] support only RISC-V machine\n");
		return -EINVAL;
	}

	if (elf->e_phoff == 0 || elf->e_phnum == 0 || elf->e_phentsize == 0) {
		log_err("[elf] no program header\n");
		return -EINVAL;
	}

	if (elf->elf_size < (elf->e_phoff + (elf->e_phnum * elf->e_phentsize))) {
		log_err("[elf] program header exceeds file size\n");
		return -EINVAL;
	}

	if (elf->elf_size < (elf->e_shoff + (elf->e_shnum * elf->e_shentsize))) {
		log_err("[elf] section header exceeds file size\n");
		return -EINVAL;
	}

	if (elf->e_shentsize == 0) {
		log_err("[elf] invalid section header\n");
		return -EINVAL;
	}

	if (elf->elf_size < (elf->e_shoff + (elf->e_shnum * elf->e_shentsize))) {
		log_err("[elf] section header exceeds file size\n");
		return -EINVAL;
	}

	if (elf->e_shstrndx >= elf->e_shnum) {
		log_err("[elf] section header string exceeds number of headers\n");
		return -EINVAL;
	}

	// entry point address
	if ((elf->e_entry % 4) != 0) {
		log_err("[elf] entry violates 4bytes alignment."
			"0x%" PRIx64 "\n", elf->e_entry);
		return -EINVAL;
	}

	return 0;
}

static int elf_read_elf32(struct elf_info *elf)
{
	const Elf32_Ehdr *elf_hdr = (const Elf32_Ehdr *)elf->buf;

	elf->e_type      = elf_hdr->e_type;
	elf->e_machine   = elf_hdr->e_machine;
	elf->e_version   = elf_hdr->e_version;
	elf->e_entry     = elf_hdr->e_entry;
	elf->e_phoff     = elf_hdr->e_phoff;
	elf->e_shoff     = elf_hdr->e_shoff;
	elf->e_flags     = elf_hdr->e_flags;
	elf->e_ehsize    = elf_hdr->e_ehsize;
	elf->e_phentsize = elf_hdr->e_phentsize;
	elf->e_phnum     = elf_hdr->e_phnum;
	elf->e_shentsize = elf_hdr->e_shentsize;
	elf->e_shnum     = elf_hdr->e_shnum;
	elf->e_shstrndx  = elf_hdr->e_shstrndx;

	if (elf->e_phentsize < sizeof(Elf32_Phdr)) {
		log_err("not support program header size %" PRId32 "\n",
			elf->e_phentsize);
		return -EINVAL;
	}

	if (elf->e_shentsize < sizeof(Elf32_Shdr)) {
		log_err("not support section header size %" PRId32 "\n",
			elf->e_shentsize);
		return -EINVAL;
	}

	return 0;
}

static int elf_read_elf64(struct elf_info *elf)
{
	const Elf64_Ehdr *elf_hdr = (const Elf64_Ehdr *)elf->buf;

	elf->e_type      = elf_hdr->e_type;
	elf->e_machine   = elf_hdr->e_machine;
	elf->e_version   = elf_hdr->e_version;
	elf->e_entry     = elf_hdr->e_entry;
	elf->e_phoff     = elf_hdr->e_phoff;
	elf->e_shoff     = elf_hdr->e_shoff;
	elf->e_flags     = elf_hdr->e_flags;
	elf->e_ehsize    = elf_hdr->e_ehsize;
	elf->e_phentsize = elf_hdr->e_phentsize;
	elf->e_phnum     = elf_hdr->e_phnum;
	elf->e_shentsize = elf_hdr->e_shentsize;
	elf->e_shnum     = elf_hdr->e_shnum;
	elf->e_shstrndx  = elf_hdr->e_shstrndx;

	if (elf->e_phentsize < sizeof(Elf64_Phdr)) {
		log_err("not support program header size %" PRId32 "\n",
			elf->e_phentsize);
		return -EINVAL;
	}

	if (elf->e_shentsize < sizeof(Elf64_Shdr)) {
		log_err("not support section header size %" PRId32 "\n",
			elf->e_shentsize);
		return -EINVAL;
	}

	return 0;
}

static int elf_read_elf(struct elf_info *elf)
{
	int r;

	switch (elf->elf_class) {
	case 32:
		r = elf_read_elf32(elf);
		break;
	case 64:
		r = elf_read_elf64(elf);
		break;
	default:
		log_err("architecture class is not 32 nor 64\n");
		return -EINVAL;
	}
	if (r) {
		return r;
	}

	elf->p_hdr_ent = elf->buf + elf->e_phoff;
	elf->s_hdr_ent = elf->buf + elf->e_shoff;

	return 0;
}

static void elf_dump_elf(const struct elf_info *elf)
{
	log_dbg("  ELF entry point: 0x%" PRIx64 "\n", elf->e_entry);
	log_dbg("  program num    : %d\n", elf->e_phnum);
	log_dbg("  program size   : %d\n", elf->e_phentsize);
	log_dbg("  program offset : 0x%" PRIx64 "\n", elf->e_phoff);
	log_dbg("  program addr   : %p\n", elf->p_hdr_ent);
}

static int elf_read_ph32(struct elf_info *elf)
{
	for (uint32_t i = 0; i < elf->e_phnum; i++) {
		const Elf32_Phdr *p_hdr = (const Elf32_Phdr *)(elf->p_hdr_ent + i * elf->e_phentsize);

		elf->phs[i].p_type   = p_hdr->p_type;
		elf->phs[i].p_flags  = p_hdr->p_flags;
		elf->phs[i].p_offset = p_hdr->p_offset;
		elf->phs[i].p_vaddr  = p_hdr->p_vaddr;
		elf->phs[i].p_paddr  = p_hdr->p_paddr;
		elf->phs[i].p_filesz = p_hdr->p_filesz;
		elf->phs[i].p_memsz  = p_hdr->p_memsz;
		elf->phs[i].p_align  = p_hdr->p_align;
	}

	return 0;
}

static int elf_read_ph64(struct elf_info *elf)
{
	for (uint32_t i = 0; i < elf->e_phnum; i++) {
		const Elf64_Phdr *p_hdr = (const Elf64_Phdr *)(elf->p_hdr_ent + i * elf->e_phentsize);

		elf->phs[i].p_type   = p_hdr->p_type;
		elf->phs[i].p_flags  = p_hdr->p_flags;
		elf->phs[i].p_offset = p_hdr->p_offset;
		elf->phs[i].p_vaddr  = p_hdr->p_vaddr;
		elf->phs[i].p_paddr  = p_hdr->p_paddr;
		elf->phs[i].p_filesz = p_hdr->p_filesz;
		elf->phs[i].p_memsz  = p_hdr->p_memsz;
		elf->phs[i].p_align  = p_hdr->p_align;
	}

	return 0;
}

static int elf_read_ph(struct elf_info *elf)
{
	struct prg_header *phs = NULL;
	int r;

	phs = calloc(elf->e_phnum, sizeof(struct prg_header));
	if (phs == NULL) {
		log_err("failed to calloc prg_header.\n");
		return -ENOMEM;
	}
	elf->phs = phs;

	switch (elf->elf_class) {
	case 32:
		r = elf_read_ph32(elf);
		break;
	case 64:
		r = elf_read_ph64(elf);
		break;
	default:
		log_err("architecture class is not 32 nor 64\n");
		return -EINVAL;
	}
	if (r) {
		return r;
	}

	return 0;
}

static void elf_dump_ph(const struct elf_info *elf)
{
	for (uint32_t i = 0; i < elf->e_phnum; i++) {
		const struct prg_header *prg = &elf->phs[i];

		log_dbg("  ph: %d, p_hdr: %p, offset: 0x%x\n",
			i, elf->p_hdr_ent + i * elf->e_phentsize, i * elf->e_phentsize);
		log_dbg("    ph type  : 0x%04x\n", prg->p_type);
		log_dbg("    ph offset: 0x%" PRIx64 "\n", prg->p_offset);
		log_dbg("    ph vaddr : 0x%" PRIx64 "\n", prg->p_vaddr);
		log_dbg("    ph paddr : 0x%" PRIx64 "\n", prg->p_paddr);
		log_dbg("    ph filesz: 0x%" PRIx64 "\n", prg->p_filesz);
		log_dbg("    ph flag  : %s%s%s (0x%04x)\n",
			(prg->p_flags & PF_W) ? "w" : "-",
			(prg->p_flags & PF_R) ? "r" : "-",
			(prg->p_flags & PF_X) ? "x" : "-",
			prg->p_flags);
		if (prg->p_type != PT_LOAD || prg->p_filesz == 0) {
			log_dbg("    do not need to load, skipped\n");
			continue;
		}
		log_dbg("  copy paddr: 0x%" PRIx64 "-0x%" PRIx64 ", size: 0x%" PRIx64 "\n",
			prg->p_paddr, prg->p_paddr + prg->p_filesz,
			prg->p_filesz);

		log_dbg("    elf hdr_ptr: %p, offset: 0x%" PRIx64 ", src: %p\n",
			elf->buf, prg->p_offset, prg->src);
	}
}

static int elf_read_shs32(struct elf_info *elf)
{
	for (uint32_t i = 0; i < elf->e_shnum; i++) {
		const Elf32_Shdr *s_hdr = (const Elf32_Shdr *)(elf->s_hdr_ent + i * elf->e_shentsize);
		elf->shs[i].sh_name      = s_hdr->sh_name;
		elf->shs[i].sh_type      = s_hdr->sh_type;
		elf->shs[i].sh_flags     = s_hdr->sh_flags;
		elf->shs[i].sh_addr      = s_hdr->sh_addr;
		elf->shs[i].sh_offset    = s_hdr->sh_offset;
		elf->shs[i].sh_size      = s_hdr->sh_size;
		elf->shs[i].sh_link      = s_hdr->sh_link;
		elf->shs[i].sh_info      = s_hdr->sh_info;
		elf->shs[i].sh_addralign = s_hdr->sh_addralign;
		elf->shs[i].sh_entsize   = s_hdr->sh_entsize;
	}

	return 0;
}

static int elf_read_shs64(struct elf_info *elf)
{
	for (uint32_t i = 0; i < elf->e_shnum; i++) {
		const Elf64_Shdr *s_hdr = (const Elf64_Shdr *)(elf->s_hdr_ent + i * elf->e_shentsize);
		elf->shs[i].sh_name      = s_hdr->sh_name;
		elf->shs[i].sh_type      = s_hdr->sh_type;
		elf->shs[i].sh_flags     = s_hdr->sh_flags;
		elf->shs[i].sh_addr      = s_hdr->sh_addr;
		elf->shs[i].sh_offset    = s_hdr->sh_offset;
		elf->shs[i].sh_size      = s_hdr->sh_size;
		elf->shs[i].sh_link      = s_hdr->sh_link;
		elf->shs[i].sh_info      = s_hdr->sh_info;
		elf->shs[i].sh_addralign = s_hdr->sh_addralign;
		elf->shs[i].sh_entsize   = s_hdr->sh_entsize;
	}

	return 0;
}

static const char *get_sh_name(const struct elf_info *elf, uint32_t off)
{
	if (elf->shstr_size <= off) {
		return "(unknown)";
	}

	return elf->shstr_ent + off;
}

static int elf_read_shs(struct elf_info *elf)
{
	struct sec_header *shs = NULL;
	int r;

	shs = calloc(elf->e_shnum, sizeof(struct sec_header));
	if (shs == NULL) {
		log_err("failed to calloc sec_header.\n");
		return -ENOMEM;
	}
	elf->shs = shs;

	switch (elf->elf_class) {
	case 32:
		r = elf_read_shs32(elf);
		break;
	case 64:
		r = elf_read_shs64(elf);
		break;
	default:
		log_err("architecture class is not 32 nor 64\n");
		return -EINVAL;
	}
	if (r) {
		return r;
	}

	struct sec_header *shstr = &elf->shs[elf->e_shstrndx];
	elf->shstr_ent = (const char *)elf->buf + shstr->sh_offset;
	elf->shstr_size = shstr->sh_size;

	return 0;
}

static const char *get_sh_type_name(uint32_t t)
{
	const char *name = "(unknown)";

	switch (t) {
	case SHT_NULL:
		name = "SHT_NULL";
		break;
	case SHT_PROGBITS:
		name = "SHT_PROGBITS";
		break;
	case SHT_SYMTAB:
		name = "SHT_SYMTAB";
		break;
	case SHT_STRTAB:
		name = "SHT_STRTAB";
		break;
	case SHT_RELA:
		name = "SHT_RELA";
		break;
	case SHT_HASH:
		name = "SHT_HASH";
		break;
	case SHT_DYNAMIC:
		name = "SHT_DYNAMIC";
		break;
	case SHT_NOTE:
		name = "SHT_NOTE";
		break;
	case SHT_NOBITS:
		name = "SHT_NOBITS";
		break;
	case SHT_REL:
		name = "SHT_REL";
		break;
	case SHT_SHLIB:
		name = "SHT_SHLIB";
		break;
	case SHT_DYNSYM:
		name = "SHT_DYNSYM";
		break;
	}

	return name;
}

static int find_shs(struct elf_info *elf, const char *name, struct sec_header **ressec)
{
	struct sec_header *sec = NULL;
	int r = -EINVAL;

	for (uint32_t i = 0; i < elf->e_shnum; i++) {
		struct sec_header *s = &elf->shs[i];

		if (strcmp(get_sh_name(elf, s->sh_name), name) == 0) {
			sec = s;
			r = 0;
			break;
		}
	}

	if (ressec) {
		*ressec = sec;
	}

	return r;
}

static void elf_dump_shs(const struct elf_info *elf)
{
	for (uint32_t i = 0; i < elf->e_shnum; i++) {
		const struct sec_header *sec = &elf->shs[i];

		log_dbg("  sh: %d, s_hdr: %p, offset: 0x%x\n",
			i, elf->s_hdr_ent + i * elf->e_shentsize, i * elf->e_shentsize);
		log_dbg("    sh name     : '%s' (0x%" PRIx32 ")\n",
			get_sh_name(elf, sec->sh_name), sec->sh_name);
		log_dbg("    sh type     : %s (0x%" PRIx32 ")\n",
			get_sh_type_name(sec->sh_type), sec->sh_type);
		log_dbg("    sh flags    : %s%s%s (0x%04" PRIx64 ")\n",
			(sec->sh_flags & SHF_WRITE)     ? "W" : "-",
			(sec->sh_flags & SHF_ALLOC)     ? "A" : "-",
			(sec->sh_flags & SHF_EXECINSTR) ? "X" : "-",
			sec->sh_flags);
		log_dbg("    sh addr     : 0x%" PRIx64 "\n", sec->sh_addr);
		log_dbg("    sh offset   : 0x%" PRIx64 "\n", sec->sh_offset);
		log_dbg("    sh size     : 0x%" PRIx64 "\n", sec->sh_size);
		log_dbg("    sh link     : 0x%" PRIx32 "\n", sec->sh_link);
		log_dbg("    sh info     : 0x%" PRIx32 "\n", sec->sh_info);
		log_dbg("    sh addralign: 0x%" PRIx64 "\n", sec->sh_addralign);
		log_dbg("    sh entsize  : 0x%" PRIx64 "\n", sec->sh_entsize);
	}
}

int elf_load(struct elf_info *elf, uint8_t *buf, size_t len)
{
	int arch;
	int r;

	r = elf_init(elf);
	if (r) {
		return r;
	}

	elf->buf = buf;
	elf->elf_size = len;

	log_dbg("  ELF addr       : %p\n", elf->buf);

	r = elf_get_class(elf->buf, &arch);
	if (r) {
		return r;
	}
	elf->elf_class = arch;

	r = elf_read_elf(elf);
	if (r) {
		return r;
	}
	elf_dump_elf(elf);

	r = elf_is_valid_header(elf);
	if (r) {
		return r;
	}

	r = elf_read_ph(elf);
	if (r) {
		return r;
	}
	elf_dump_ph(elf);

	r = elf_read_shs(elf);
	if (r) {
		return r;
	}
	elf_dump_shs(elf);

	return 0;
}

int elf_unload(struct elf_info *elf)
{
	int r, res = 0;

	r = elf_fini(elf);
	if (r) {
		// Continue anyway
		res = r;
	}

	return res;
}

int embed_auxv(const char *fname, uint8_t *bin, size_t bin_sz)
{
	struct elf_info elf;
	struct sec_header *sec;
	struct __aux_data aux;
	int r;

	r = elf_load(&elf, bin, bin_sz);
	if (r) {
		log_err("failed to elf_load.\n");
		return -EINVAL;
	}

	r = find_shs(&elf, ".auxdata", &sec);
	if (r) {
		log_err("failed to find .auxdata section.\n");
		return -EINVAL;
	}

	aux.valid = 1;
	aux.phent = elf.e_phentsize;
	aux.phnum = elf.e_phnum;
	aux.phdr_size = elf.e_phentsize * elf.e_phnum;

	uint8_t *dest = elf.buf + sec->sh_offset;
	size_t sz_req = sizeof(aux) + aux.phdr_size;

	if (sec->sh_size < sz_req) {
		log_err(".auxdata (%d bytes) is too small, needs %d bytes.\n",
			(int)sec->sh_size, (int)sz_req);
		return -ENOMEM;
	}
	log_info("%s: Write %d bytes into .auxdata (%d bytes) section.\n",
		fname, (int)sz_req, (int)sec->sh_size);

	memcpy(dest, &aux, sizeof(aux));
	dest += sizeof(aux);
	memcpy(dest, elf.p_hdr_ent, aux.phdr_size);

	r = elf_unload(&elf);
	if (r) {
		log_err("failed to elf_unload.\n");
		return -EINVAL;
	}

	return 0;
}

int embed_file(const char *fname)
{
	int fd, r;
	struct stat st;
	uint8_t *bin;
	size_t bin_sz;

	fd = open(fname, O_RDWR);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	r = fstat(fd, &st);
	if (r != 0) {
		perror("fstat");
		return r;
	}

	bin_sz = st.st_size;
	bin = mmap(NULL, bin_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (bin == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	r = embed_auxv(fname, bin, bin_sz);
	if (r) {
		log_err("%s: failed to embed_auxv.\n", fname);
		return r;
	}

	munmap(bin, bin_sz);
	close(fd);

	return 0;
}

int main(int argc, char *argv[])
{
	int r, ret = 0;

	if (argc == 1) {
		return embed_file("a.out");
	}

	for (int i = 1; i < argc; i++) {
		r = embed_file(argv[i]);
		if (r) {
			ret = r;
		}
	}

	return ret;
}
