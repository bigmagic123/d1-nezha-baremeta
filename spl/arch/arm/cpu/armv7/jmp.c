// * SPDX-License-Identifier:	GPL-2.0+

#include <common.h>

void boot0_jmp(phys_addr_t addr)
{
    asm volatile("mov r2, #0");
    asm volatile("mcr p15, 0, r2, c7, c5, 6");
    asm volatile("bx r0");
}

void boot0_jmp_optee(phys_addr_t optee, phys_addr_t uboot)
{
	phys_addr_t optee_entry = optee;
	phys_addr_t uboot_entry = uboot;

	/* dtb_entry is fixed to 2M offset of uboot_entry */
	phys_addr_t dtb_entry = uboot_entry + 2 * 1024 * 1024;

	asm volatile ("mov r2, %0" : : "r" (dtb_entry) : "memory");
	asm volatile ("mov lr, %0" : : "r" (uboot_entry) : "memory");
	asm volatile ("bx      %0" : : "r" (optee_entry) : "memory");
}

void boot0_jmp_monitor(phys_addr_t monitor_base)
{
#ifdef CONFIG_MONITOR
	/* jmp to AA64
	 *set the cpu boot entry addr:
	 */
	writel(monitor_base,RVBARADDR0_L);
	writel(0,RVBARADDR0_H);

	/*set cpu to AA64 execution state when the cpu boots into after a warm reset*/
	asm volatile("MRC p15,0,r2,c12,c0,2");
	asm volatile("ORR r2,r2,#(0x3<<0)");
	asm volatile("DSB");
	asm volatile("MCR p15,0,r2,c12,c0,2");
	asm volatile("ISB");
__LOOP:
	asm volatile("WFI");
	goto __LOOP;
#endif
}

void boot0_jmp_opensbi(phys_addr_t opensbi_base, phys_addr_t uboot_base)
{
__LOOP:
	asm volatile("WFI");
	goto __LOOP;
}

