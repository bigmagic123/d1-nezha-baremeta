/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>


void enable_smp(void)
{
	__maybe_unused u32 val;

#ifndef CFG_SUNXI_ARM64
	/* SMP status is controlled by bit 6 of the CP15 Aux Ctrl Reg:ACTLR */
	asm volatile("MRC     p15, 0, r0, c1, c0, 1");
	asm volatile("ORR     r0, r0, #0x040");
	asm volatile("MCR     p15, 0, r0, c1, c0, 1");
	#ifdef DEBUG_MMU
		asm volatile("MRC	  p15, 0, r0, c1, c0, 1");
		asm volatile("mov %0, r0" : "=r"(val));
		printf("val:%x\n", val);
	#endif
#else
	asm volatile("MRRC p15,1,r0,r1,c15");
	#ifdef DEBUG_MMU
		asm volatile("mov %0, r0" : "=r"(val));
		printf("val:%x\n", val);
	#endif
	asm volatile("ORR     r0, r0, #0x040");
	asm volatile("MCRR p15,1,r0,r1,c15");
	#ifdef DEBUG_MMU
		asm volatile("mov %0, r0" : "=r"(val));
		printf("val:%x\n", val);
	#endif
#endif
}

#ifdef CFG_USE_DCACHE
__weak void dcache_enable(void)
{
	u32 reg;
	/* and enable the mmu */
	asm volatile("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (reg) : : "cc");

	udelay(100);
	reg |= (1 << 2);    //enable  dcache
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (reg) : "cc");
	asm volatile("isb");

}

__weak void dcache_disable(void)
{

	u32 reg;
	asm volatile("blx %0" : : "r" (v7_flush_dcache_all));
	asm volatile("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (reg) : : "cc");
	udelay(100);
	reg &= ~((1<<2));    //disable dcache
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (reg) : "cc");
	asm volatile("isb");
}
#endif



__weak void mmu_enable(u32 dram_size)
{
	u32 mmu_base;

	/*use dram high 16M*/
	if (dram_size > 2048)
		dram_size = 2048;
	u32* mmu_base_addr = (u32 *)(CONFIG_SYS_DRAM_BASE +((dram_size-1)<<20));
	u32* page_table = mmu_base_addr;

	int i;
	u32 reg;

	/* the front 1M contain BROM/SRAM */
#ifdef CFG_USE_DCACHE
	page_table[0] = (3 << 10) | (15 << 5) | (1 << 3) | (1 << 2) | 0x2;
#else
	page_table[0] = (3 << 10) | (15 << 5) | (1 << 3) | (0 << 2) | 0x2;
#endif
	/* the front 1G of memory(treated as 4G for all) is set up as none cacheable */
	for (i = 1; i < (CONFIG_SYS_DRAM_BASE>>20); i++)
		page_table[i] = (i << 20) | (3 << 10) | (15 << 5) | (0 << 3) | 0x2;
	/* Set up as write back and buffered for other 3GB, rw for everyone */
	for (i = (CONFIG_SYS_DRAM_BASE>>20); i < 4096; i++)
#ifdef CFG_USE_DCACHE
		page_table[i] = (i << 20) | (3 << 10) | (15 << 5) | (1 << 3) | (1 << 2) | 0x2;
#else
		page_table[i] = (i << 20) | (3 << 10) | (15 << 5) | (1 << 3) | (0 << 2) | 0x2;
#endif
	/* flush tlb */
	asm volatile("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
	/* Copy the page table address to cp15 */

	mmu_base = (u32)mmu_base_addr;
	mmu_base |= (1 << 0) | (1 << 1) | (2 << 3);
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (mmu_base) : "memory");
	asm volatile("mcr p15, 0, %0, c2, c0, 1"
		     : : "r" (mmu_base) : "memory");
	/* Set the access control to all-supervisor */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (0x55555555));			//modified, origin value is (~0)
	asm volatile("isb");
#ifdef CFG_USE_DCACHE
	/* enable smp for dcache */
	enable_smp();
#endif
	/* and enable the mmu */
	asm volatile("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (reg) : : "cc");

	udelay(100);
	reg |= ((1<<0)|(1<<12));    //enable mmu, icache
	reg &= ~(1 << 2);			//disable dcache
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (reg) : "cc");
	asm volatile("isb");
}

__weak void mmu_disable(void)
{
	uint reg;
#ifdef CFG_USE_DCACHE
	/* flush dcache */
	asm volatile("blx %0" : : "r" (v7_flush_dcache_all));
#endif
	/* and disable the mmu */
	asm volatile("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (reg) : : "cc");
	udelay(100);
	reg &= ~((7<<0)|(1<<12));    //disable mmu, icache, dcache
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (reg) : "cc");
	asm volatile("isb");
	/*
	 * Invalidate all instruction caches to PoU.
	 * Also flushes branch target cache.
	 */
	asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));
	/* Invalidate entire branch predictor array */
	asm volatile ("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));
	/* Full system DSB - make sure that the invalidation is complete */
	asm volatile("dsb");
	/* ISB - make sure the instruction stream sees it */
	asm volatile("isb");
}

void data_sync_barrier(void)
{
	asm volatile("DSB");
	asm volatile("ISB");
}

void neon_enable(void)
{
	/*
	 * multiple optional feature use neon
	 * let them do their neon enable as they need
	 * and block redundant enabel here
	 */
	static int neon_enabled;
	if (neon_enabled) {
		return;
	}
	neon_enabled = 1;

	/* set NSACR, both Secure and Non-secure access are allowed to NEON */
	asm volatile("MRC p15, 0, r0, c1, c1, 2");
	asm volatile("ORR r0, r0, #(0x3<<10) @ enable fpu/neon");
	asm volatile("MCR p15, 0, r0, c1, c1, 2");
	/* Set the CPACR for access to CP10 and CP11*/
	asm volatile("LDR r0, =0xF00000");
	asm volatile("MCR p15, 0, r0, c1, c0, 2");
	/* Set the FPEXC EN bit to enable the FPU */
	asm volatile("MOV r3, #0x40000000");
	asm volatile("MCR p10, 7, r3, c8, c0, 0");
}

void invalidate_dcache_range(__maybe_unused unsigned long start,
			     __maybe_unused unsigned long stop) { }

void flush_dcache_range(__maybe_unused unsigned long start,
			__maybe_unused unsigned long stop) { }
