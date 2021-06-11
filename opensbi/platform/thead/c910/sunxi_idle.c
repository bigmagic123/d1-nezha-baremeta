/*
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sbi/riscv_encoding.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_string.h>
#include "sunxi_platform.h"
#include "sunxi_cpuidle.h"

//#define SUNXI_RESET_ENTRY
//#define DEBUG_IDLE
#ifdef DEBUG_IDLE
#define db_info( fmt,  arg  ... )  \
		do { sbi_printf("[DEBUG] " fmt ,  ##arg );  } while(0);
#else
#define db_info( fmt,  arg  ... )  
#endif
#define db_err( fmt,  arg  ... )  \
		do { sbi_printf("[err] " fmt ,  ##arg );  } while(0);

struct sbi_power_operations {
	char name[20];
	/** env save  */
	void (*power_down)(void);
	/** env restore */
	void (*power_up)(void);
};

unsigned long SBI_GPRS[SBI_GPR_MAX];
unsigned long C906_EXTEN_CSR[C906_EXTEN_MAX];
unsigned long SBI_PMP[SBI_PMP_MAX];
unsigned long SBI_MCSR[SBI_CSR_MMAX];
unsigned long SBI_SCSR[SBI_CSR_SMAX];


extern void sbi_system_resume(unsigned long addr);
extern void sbi_suspend_finish();
extern struct c910_regs_struct c910_regs;

void sbi_plic_save()
{
}

void sbi_plic_restore()
{
	writel(C910_PLIC_DELEG_ENABLE,
		(void *)c910_regs.plic_base_addr + C910_PLIC_DELEG_OFFSET);
}

void c906_save_extension_csrs()
{
	C906_EXTEN_CSR[C906_EXTEN(mcor)] = csr_read(CSR_MCOR);
	C906_EXTEN_CSR[C906_EXTEN(mhcr)] = csr_read(CSR_MHCR);
	C906_EXTEN_CSR[C906_EXTEN(mhint)] = csr_read(CSR_MHINT);
	C906_EXTEN_CSR[C906_EXTEN(mxstatus)] = csr_read(CSR_MXSTATUS);
}

void c906_restore_extension_csrs()
{
	csr_write(CSR_MCOR,C906_EXTEN_CSR[C906_EXTEN(mcor)]);
	csr_write(CSR_MHCR,C906_EXTEN_CSR[C906_EXTEN(mhcr)]);
	csr_write(CSR_MHINT,C906_EXTEN_CSR[C906_EXTEN(mhint)]);
	csr_write(CSR_MXSTATUS,C906_EXTEN_CSR[C906_EXTEN(mxstatus)]);
}

void sbi_csrs_save()
{
	SBI_MCSR[CSR_M(minstret)] = csr_read(CSR_MINSTRET);
	SBI_MCSR[CSR_M(mstatus)] = csr_read(CSR_MSTATUS);
	SBI_MCSR[CSR_M(medeleg)] = csr_read(CSR_MEDELEG);
	SBI_MCSR[CSR_M(mideleg)] = csr_read(CSR_MIDELEG);
	SBI_MCSR[CSR_M(mtvec)] = csr_read(CSR_MTVEC);
	SBI_MCSR[CSR_M(mie)] = csr_read(CSR_MIE);
	SBI_MCSR[CSR_M(mcounteren)] = csr_read(CSR_MCOUNTEREN);
	SBI_MCSR[CSR_M(mscratch)] = csr_read(CSR_MSCRATCH);
	SBI_MCSR[CSR_M(mepc)] = csr_read(CSR_MEPC);
	SBI_MCSR[CSR_M(mcause)] = csr_read(CSR_MCAUSE);
	SBI_MCSR[CSR_M(mcounterwen)] = csr_read(CSR_MCOUNTERWEN);
	//	SBI_MCSR[CSR_M(mcounterinten)] = csr_read(CSR_MCOUNTERINTEN);
	SBI_MCSR[CSR_M(mcycle)] = csr_read(CSR_MCYCLE);

	SBI_SCSR[CSR_S(satp)] = csr_read(CSR_SATP);
	SBI_SCSR[CSR_S(sstatus)] = csr_read(CSR_SSTATUS);
	SBI_SCSR[CSR_S(sie)] = csr_read(CSR_SIE);
	SBI_SCSR[CSR_S(stvec)] = csr_read(CSR_STVEC);
	SBI_SCSR[CSR_S(scounteren)] = csr_read(CSR_SCOUNTEREN);
	SBI_SCSR[CSR_S(sscratch)] = csr_read(CSR_SSCRATCH);
	SBI_SCSR[CSR_S(sepc)] = csr_read(CSR_SEPC);
	SBI_SCSR[CSR_S(scause)] = csr_read(CSR_SCAUSE);
	//	SBI_SCSR[CSR_S(sxstatus     )] = csr_read(CSR_SXSTATUS     );
	//	SBI_SCSR[CSR_S(shcr         )] = csr_read(CSR_SHCR         );
	//	SBI_SCSR[CSR_S(scounterinten)] = csr_read(CSR_SCOUNTERINTEN);
	//	SBI_SCSR[CSR_S(scycle       )] = csr_read(CSR_SCYCLE       );
	SBI_SCSR[CSR_S(smir)] = csr_read(CSR_SMIR);
	SBI_SCSR[CSR_S(smel)] = csr_read(CSR_SMEL);
	SBI_SCSR[CSR_S(smeh)] = csr_read(CSR_SMEH);
	SBI_SCSR[CSR_S(smcir)] = csr_read(CSR_SMCIR);
}

void sbi_csrs_restore()
{
	csr_write(CSR_MINSTRET, SBI_MCSR[CSR_M(minstret)]);
	csr_write(CSR_MSTATUS, SBI_MCSR[CSR_M(mstatus)]);
	csr_write(CSR_MEDELEG, SBI_MCSR[CSR_M(medeleg)]);
	csr_write(CSR_MIDELEG, SBI_MCSR[CSR_M(mideleg)]);
	csr_write(CSR_MTVEC, SBI_MCSR[CSR_M(mtvec)]);
	csr_write(CSR_MIE, SBI_MCSR[CSR_M(mie)]);
	csr_write(CSR_MCOUNTEREN, SBI_MCSR[CSR_M(mcounteren)]);
	csr_write(CSR_MSCRATCH, SBI_MCSR[CSR_M(mscratch)]);
	csr_write(CSR_MEPC, SBI_MCSR[CSR_M(mepc)]);
	csr_write(CSR_MCAUSE, SBI_MCSR[CSR_M(mcause)]);
	csr_write(CSR_MCOUNTERWEN, SBI_MCSR[CSR_M(mcounterwen)]);
	//	csr_write(CSR_MCOUNTERINTEN,SBI_MCSR[CSR_M(mcounterinten)] );
	csr_write(CSR_MCYCLE, SBI_MCSR[CSR_M(mcycle)]);

	csr_write(CSR_SATP, SBI_SCSR[CSR_S(satp)]);
	csr_write(CSR_SSTATUS, SBI_SCSR[CSR_S(sstatus)]);
	csr_write(CSR_SIE, SBI_SCSR[CSR_S(sie)]);
	csr_write(CSR_STVEC, SBI_SCSR[CSR_S(stvec)]);
	csr_write(CSR_SCOUNTEREN, SBI_SCSR[CSR_S(scounteren)]);
	csr_write(CSR_SSCRATCH, SBI_SCSR[CSR_S(sscratch)]);
	csr_write(CSR_SEPC, SBI_SCSR[CSR_S(sepc)]);
	csr_write(CSR_SCAUSE, SBI_SCSR[CSR_S(scause)]);
	//	csr_write(CSR_SXSTATUS     ,SBI_SCSR[CSR_S(sxstatus     )] );
	//	csr_write(CSR_SHCR         ,SBI_SCSR[CSR_S(shcr         )] );
	//	csr_write(CSR_SCOUNTERINTEN,SBI_SCSR[CSR_S(scounterinten)] );
	//	csr_write(CSR_SCYCLE       ,SBI_SCSR[CSR_S(scycle       )] );
	csr_write(CSR_SMIR, SBI_SCSR[CSR_S(smir)]);
	csr_write(CSR_SMEL, SBI_SCSR[CSR_S(smel)]);
	csr_write(CSR_SMEH, SBI_SCSR[CSR_S(smeh)]);
	csr_write(CSR_SMCIR, SBI_SCSR[CSR_S(smcir)]);
}

void sbi_pmp_save()
{
	SBI_PMP[PMP(addr0)] = csr_read(CSR_PMPADDR0);
	SBI_PMP[PMP(addr1)] = csr_read(CSR_PMPADDR1);
	SBI_PMP[PMP(addr2)] = csr_read(CSR_PMPADDR2);
	SBI_PMP[PMP(addr3)] = csr_read(CSR_PMPADDR3);
	SBI_PMP[PMP(addr4)] = csr_read(CSR_PMPADDR4);
	SBI_PMP[PMP(addr5)] = csr_read(CSR_PMPADDR5);
	SBI_PMP[PMP(addr6)] = csr_read(CSR_PMPADDR6);
	SBI_PMP[PMP(addr7)] = csr_read(CSR_PMPADDR7);
	SBI_PMP[PMP(cfg0)] = csr_read(CSR_PMPCFG0);

}

void sbi_pmp_restore() 
{
	csr_write(CSR_PMPADDR0, SBI_PMP[PMP(addr0)]);
	csr_write(CSR_PMPADDR1, SBI_PMP[PMP(addr1)]);
	csr_write(CSR_PMPADDR2, SBI_PMP[PMP(addr2)]);
	csr_write(CSR_PMPADDR3, SBI_PMP[PMP(addr3)]);
	csr_write(CSR_PMPADDR4, SBI_PMP[PMP(addr4)]);
	csr_write(CSR_PMPADDR5, SBI_PMP[PMP(addr5)]);
	csr_write(CSR_PMPADDR6, SBI_PMP[PMP(addr6)]);
	csr_write(CSR_PMPADDR7, SBI_PMP[PMP(addr7)]);
	csr_write(CSR_PMPCFG0, SBI_PMP[PMP(cfg0)]);
}

void sbi_ppu_set_entry(unsigned long addr)
{
	unsigned long entry = addr;
	db_info("entry:0x%lx\n",entry);
#ifdef SUNXI_RESET_ENTRY
	writel((entry & 0xFFFFFFFF), (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_RESET_ENTRY_LOW));
	writel((entry >> 32), (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_RESET_ENTRY_LOW));
#else
	writel(HOTPLUG_FLAG_VAL, (void *)HOTPLUG_FLAG_REG);
	writel((entry & 0xFFFFFFFF), (void *)HOTPLUG_SOFTENTRY_REG);

#endif
}

void sbi_power_system_init(void)
{
	/* config riscv cfg gating */
	writel(0x10001,(void *)(SUNXI_CCU_BASE + SUNXI_RISCV_CFG_BGR_REG));
	/* config ppu gating */
	writel(0x10001,(void *)(SUNXI_RPRCM_BASE + SUNXI_RISCV_PPU_BUS_GATING));

	/* unmask irq signal  */
	writel(0xFFFFFFFF,(void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK0_REG));
	writel(0xFFFFFFFF,(void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK1_REG));
	writel(0xFFFFFFFF,(void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK2_REG));
	writel(0xFFFFFFFF,(void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK3_REG));
	writel(0xFFFFFFFF,(void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK4_REG));
}

static void sbi_ppu_active_en(int state)
{
	if (state) {

		/* config power switch */
		sbi_power_system_init();
		/* prepare resume entry */
		sbi_ppu_set_entry((unsigned long)sbi_system_resume);

		writel(1, (void *)(SUNXI_PPU_BASE + SUNXI_PD_ACTIVE_CTRL));
		writel(1, (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WAKEUP_EN_REG));
	} else {
		writel(0, (void *)(SUNXI_PPU_BASE + SUNXI_PD_ACTIVE_CTRL));
		writel(0, (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WAKEUP_EN_REG));

		/* unmask irq signal  */
		writel(0,
		       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK0_REG));
		writel(0,
		       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK1_REG));
		writel(0,
		       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK2_REG));
		writel(0,
		       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK3_REG));
		writel(0,
		       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK4_REG));
	}

}

static const struct sbi_power_operations power_ops[] = {
	{ "PLIC", sbi_plic_save, sbi_plic_restore },
	{ "PERF", c906_save_extension_csrs, c906_restore_extension_csrs },
	{ "CSRS", sbi_csrs_save, sbi_csrs_restore },
	{ "PMP", sbi_pmp_save, sbi_pmp_restore }
};

static int sbi_power_operation(char *name, int state)
{
	struct sbi_power_operations *op;

	for (op = (struct sbi_power_operations *) power_ops; op != NULL; op++) {
		if (sbi_memcmp(op->name, name, sbi_strlen(name)) != 0)
			continue;
		else
			break;
	}

	if (!op) {
		db_err("No %s operation.\n",name);
		return 0;
	}

	if (state) {
		if (op->power_up)
			op->power_up();

	} else {
		if (op->power_down)
			op->power_down();
	}
	return 0;
}

void load_standby_elf(void);
static unsigned int wakeup_mask[5];

static void sbi_system_wakeup_set(void)
{
	writel(wakeup_mask[0],
	       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK0_REG));
	writel(wakeup_mask[1],
	       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK1_REG));
	writel(wakeup_mask[2],
	       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK2_REG));
	writel(wakeup_mask[3],
	       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK3_REG));
	writel(wakeup_mask[4],
	       (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_WKAEUP_MASK4_REG));
	sbi_printf("%x,%x,%x,%x,%x\n", wakeup_mask[0], wakeup_mask[1],
		   wakeup_mask[2], wakeup_mask[3], wakeup_mask[4]);
}

/*
 * @state: standby if state is zero, others is idle
 */
void sbi_system_suspend(int state)
{
	if (state > 2) {
		db_err("cpu suspend: err state.\n");
	}

	db_info("enter idle.\n");
	sbi_power_operation("PLIC", 0);
	sbi_power_operation("PERF", 0);
	sbi_power_operation("CSRS", 0);
	sbi_power_operation("PMP", 0);
	sbi_ppu_active_en(1);
	if (state) {
		sbi_suspend_finish(SBI_GPRS);
	} else {
		sbi_system_wakeup_set();
		load_standby_elf();
	}
	sbi_power_operation("PERF", 1);
	sbi_ppu_active_en(0);
	sbi_power_operation("CSRS", 1);
	sbi_power_operation("PMP", 1);
	sbi_power_operation("PLIC", 1);
	db_info("wake up.\n");
}

void sbi_system_set_wakeup(unsigned long irq, unsigned long on)
{
	int irqs = sizeof(wakeup_mask) * 32;

	if (irq < 16)
		return;

	irq -= 16;
	if (irq >= irqs)
		return;

	if (on)
		wakeup_mask[irq / 32] |= 1 << (irq % 32);
	else
		wakeup_mask[irq / 32] &= ~(1 << (irq % 32));
}


