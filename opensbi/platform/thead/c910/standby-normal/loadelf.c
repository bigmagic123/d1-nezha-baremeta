// SPDX-License-Identifier: BSD-2-Clause
#include <stdint.h>
#include <sbi/sbi_console.h>
#include <sbi/riscv_asm.h>
#include "../sunxi_platform.h"

/* 64-bit ELF base types. */
typedef uint64_t	Elf64_Addr;
typedef uint16_t	Elf64_Half;
typedef int16_t		Elf64_SHalf;
typedef uint64_t	Elf64_Off;
typedef int32_t		Elf64_Sword;
typedef uint32_t	Elf64_Word;
typedef uint64_t	Elf64_Xword;
typedef int64_t		Elf64_Sxword;

#define EI_NIDENT 16

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7               /* Thread local storage segment */
#define PT_LOOS    0x60000000      /* OS-specific */
#define PT_HIOS    0x6fffffff      /* OS-specific */
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff
#define PT_GNU_EH_FRAME		0x6474e550
#define PT_GNU_PROPERTY		0x6474e553

#define PT_GNU_STACK	(PT_LOOS + 0x474e551)
typedef struct elf64_hdr {
  unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" */
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;		/* Entry point virtual address */
  Elf64_Off e_phoff;		/* Program header table file offset */
  Elf64_Off e_shoff;		/* Section header table file offset */
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

/* These constants define the permissions on sections in the program
   header, p_flags. */
#define PF_R		0x4
#define PF_W		0x2
#define PF_X		0x1

typedef struct elf64_phdr {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;		/* Segment file offset */
  Elf64_Addr p_vaddr;		/* Segment virtual address */
  Elf64_Addr p_paddr;		/* Segment physical address */
  Elf64_Xword p_filesz;		/* Segment size in file */
  Elf64_Xword p_memsz;		/* Segment size in memory */
  Elf64_Xword p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;

/* Generate getter and setter for a specific elf struct/field */
#define ELF_GEN_FIELD_GET_SET(__s, __field, __type) \
static inline __type elf_##__s##_get_##__field(const void *arg) \
{ \
	return (__type) ((const struct elf64_##__s *) arg)->__field; \
} \
static inline void elf_##__s##_set_##__field(void *arg, \
					     __type value) \
{ \
	((struct elf64_##__s *) arg)->__field = (__type) value; \
}

ELF_GEN_FIELD_GET_SET(hdr, e_entry, uint64_t)
ELF_GEN_FIELD_GET_SET(hdr, e_phnum, uint16_t)
ELF_GEN_FIELD_GET_SET(hdr, e_shnum, uint16_t)
ELF_GEN_FIELD_GET_SET(hdr, e_phoff, uint64_t)
ELF_GEN_FIELD_GET_SET(hdr, e_shoff, uint64_t)
ELF_GEN_FIELD_GET_SET(hdr, e_shstrndx, uint16_t)
ELF_GEN_FIELD_GET_SET(hdr, e_machine, uint16_t)
ELF_GEN_FIELD_GET_SET(hdr, e_type, uint16_t)
ELF_GEN_FIELD_GET_SET(hdr, e_version, uint32_t)
ELF_GEN_FIELD_GET_SET(hdr, e_ehsize, uint32_t)
ELF_GEN_FIELD_GET_SET(hdr, e_phentsize, uint16_t)

ELF_GEN_FIELD_GET_SET(phdr, p_paddr, uint64_t)
ELF_GEN_FIELD_GET_SET(phdr, p_vaddr, uint64_t)
ELF_GEN_FIELD_GET_SET(phdr, p_filesz, uint64_t)
ELF_GEN_FIELD_GET_SET(phdr, p_memsz, uint64_t)
ELF_GEN_FIELD_GET_SET(phdr, p_type, uint32_t)
ELF_GEN_FIELD_GET_SET(phdr, p_offset, uint64_t)
ELF_GEN_FIELD_GET_SET(phdr, p_flags, uint32_t)
ELF_GEN_FIELD_GET_SET(phdr, p_align, uint64_t)

#define ELF_STRUCT_SIZE(__s) \
static inline unsigned long elf_size_of_##__s(void) \
{ \
	return sizeof(struct elf64_##__s); \
}

ELF_STRUCT_SIZE(phdr)
ELF_STRUCT_SIZE(hdr)

static void *memset(void *s, int c, size_t count)
{
	char *xs = s;
	while (count--)
		*xs++ = c;
	return s;
}

static void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}

static void load_riscv64_elf(void *elf_head)
{
	const void *ehdr, *phdr;
	int i;
	uint16_t phnum;
	const uint8_t *elf_data = elf_head;
	uint32_t elf_phdr_get_size = elf_size_of_phdr();

	ehdr = elf_data;
	phnum = elf_hdr_get_e_phnum(ehdr);
	phdr = elf_data + elf_hdr_get_e_phoff(ehdr);

	/* go through the available ELF segments */
	for (i = 0; i < phnum; i++, phdr += elf_phdr_get_size) {
		uint64_t da = elf_phdr_get_p_paddr(phdr);
		uint64_t memsz = elf_phdr_get_p_memsz(phdr);
		uint64_t filesz = elf_phdr_get_p_filesz(phdr);
		uint64_t offset = elf_phdr_get_p_offset(phdr);
		uint32_t type = elf_phdr_get_p_type(phdr);
		void *ptr;

		if (type != PT_LOAD)
			continue;

		if (filesz > memsz) {
			sbi_printf("bad phdr filesz 0x%lx memsz 0x%lx\n",
				filesz, memsz);
			return;
		}

		ptr = (void *)da;

		/* put the segment where the remote processor expects it */
		if (filesz)
			memcpy(ptr, elf_data + offset, filesz);

		/*
		 * Zero out remaining memory for this segment.
		 *
		 * This isn't strictly required since dma_alloc_coherent already
		 * did this for us. albeit harmless, we may consider removing
		 * this.
		 */
		if (memsz > filesz)
			memset(ptr + filesz, 0, memsz - filesz);
	}
}
#define DBG_STANDBY

static void run_riscv64_elf(void *elf_head)
{
#ifdef DBG_STANDBY
	uint32_t *p = (void *)0x20038;
	uint32_t i ;

	for (i = 0; i < 32; i++) {
		sbi_printf("dram %x\n", *p++);
	}
#endif
	 int (*entry)(void) = (void *)elf_hdr_get_e_entry(elf_head);

	 entry();
}

extern char *standby_bin_start;
extern char *standby_bin_end;


#define MIE_SEIE (1 << 9)
#define MIE_STIE (1 << 5)
#define MIE_MTIE (1 << 7)
#define MIE_SSIE (1 << 1)

#define CSR_MHCRA         0x7c1
static void copy_paras_to_sram(void);
void load_standby_elf(int level)
{
	uint64_t mie_reg;
	uint64_t mhcr;

	mhcr = csr_read(CSR_MHCRA);
	asm volatile ("dcache.call");
	csr_write(CSR_MHCRA, 0);

	mie_reg = csr_read_clear(mie, (MIE_STIE | MIE_MTIE));
	load_riscv64_elf(&standby_bin_start);
	copy_paras_to_sram();
	run_riscv64_elf(&standby_bin_start);
	csr_write(mie, mie_reg);

	csr_write(CSR_MHCRA, mhcr);
}

static unsigned int time_to_wakeup_ms = 0;
static unsigned int arisc_debug_dram_crc_en = 0;
static unsigned int arisc_debug_dram_crc_srcaddr = 0x40020000;
static unsigned int arisc_debug_dram_crc_len = (1024 * 1024);
int sbi_set_wakeup_src_timer(uint32_t wakeup_irq)
{
	if (((wakeup_irq >> 30) & 0x03) == 0x03)
		time_to_wakeup_ms = wakeup_irq & (~(3 << 30));

	return 0;
}

int sbi_set_dram_crc_paras(long dram_crc_en, long dram_crc_srcaddr,
			   long dram_crc_len)
{
	arisc_debug_dram_crc_len = dram_crc_len;
	arisc_debug_dram_crc_srcaddr = dram_crc_srcaddr;
	arisc_debug_dram_crc_en = dram_crc_en;

	return 0;
}

/*
 * this struct must be the same as sram standby code
 */
typedef struct sram_head {
	uint32_t entry;
	uint32_t crc_start;
	uint32_t crc_len;
	uint32_t crc_before;
	uint32_t crc_after;
	uint32_t time_to_wakeup_ms;
	uint32_t crc_enable;
} sram_head;

static void copy_paras_to_sram(void)
{
	struct sram_head *p = (void *)SUNXI_SRAM_ADDR;

	p->crc_len = arisc_debug_dram_crc_len;
	p->crc_start = arisc_debug_dram_crc_srcaddr;
	p->crc_enable = arisc_debug_dram_crc_en;
	p->time_to_wakeup_ms = time_to_wakeup_ms;
}
