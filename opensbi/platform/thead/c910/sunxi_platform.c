/*
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sbi/riscv_encoding.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_console.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/sys/clint.h>
#include <sbi_utils/serial/sunxi-uart.h>
#include "sunxi_platform.h"
#include "private_opensbi.h"
#include "sbi/sbi_ecall_interface.h"

#define SBI_SET_WAKEUP_TIMER          (SBI_EXT_VENDOR_START + 0x1000)
#define SBI_SET_DEBUG_LEVEL             (SBI_EXT_VENDOR_START + 0x1001)
#define SBI_SET_DEBUG_DRAM_CRC_PARAS    (SBI_EXT_VENDOR_START + 0x1002)
#define SBI_SET_UART_BAUDRATE           (SBI_EXT_VENDOR_START + 0x1003)

extern struct private_opensbi_head  opensbi_head;
struct c910_regs_struct c910_regs;
extern int sbi_set_wakeup_src_timer(uint32_t wakeup_irq);
extern int sbi_set_dram_crc_paras(long dram_crc_en, long dram_crc_srcaddr,
				  long dram_crc_len);

static void c910_delegate_traps()
{
	unsigned long exceptions = csr_read(CSR_MEDELEG);

	/* Delegate 0 ~ 7 exceptions to S-mode */
	exceptions |= ((1U << CAUSE_MISALIGNED_FETCH) | (1U << CAUSE_FETCH_ACCESS) |
		(1U << CAUSE_ILLEGAL_INSTRUCTION) | (1U << CAUSE_BREAKPOINT) |
		(1U << CAUSE_MISALIGNED_LOAD) | (1U << CAUSE_LOAD_ACCESS) |
		(1U << CAUSE_MISALIGNED_STORE) | (1U << CAUSE_STORE_ACCESS));
	csr_write(CSR_MEDELEG, exceptions);
}

static int c910_early_init(bool cold_boot)
{
	unsigned long addr = 0;
		addr = opensbi_head.dtb_base;
		sbi_printf("opensbi r: %ld\n", addr);
	if (cold_boot) {
		/* Load from boot core */
		c910_regs.pmpaddr0 = csr_read(CSR_PMPADDR0);
		c910_regs.pmpaddr1 = csr_read(CSR_PMPADDR1);
		c910_regs.pmpaddr2 = csr_read(CSR_PMPADDR2);
		c910_regs.pmpaddr3 = csr_read(CSR_PMPADDR3);
		c910_regs.pmpaddr4 = csr_read(CSR_PMPADDR4);
		c910_regs.pmpaddr5 = csr_read(CSR_PMPADDR5);
		c910_regs.pmpaddr6 = csr_read(CSR_PMPADDR6);
		c910_regs.pmpaddr7 = csr_read(CSR_PMPADDR7);
		c910_regs.pmpcfg0  = csr_read(CSR_PMPCFG0);

		c910_regs.mcor     = csr_read(CSR_MCOR);
		c910_regs.mhcr     = csr_read(CSR_MHCR);
		c910_regs.mccr2    = csr_read(CSR_MCCR2);
		c910_regs.mhint    = csr_read(CSR_MHINT);
		c910_regs.mxstatus = csr_read(CSR_MXSTATUS);

		c910_regs.plic_base_addr = csr_read(CSR_PLIC_BASE);
		c910_regs.clint_base_addr =
			c910_regs.plic_base_addr + C910_PLIC_CLINT_OFFSET;
	} else {
		/* Store to other core */
		csr_write(CSR_PMPADDR0, c910_regs.pmpaddr0);
		csr_write(CSR_PMPADDR1, c910_regs.pmpaddr1);
		csr_write(CSR_PMPADDR2, c910_regs.pmpaddr2);
		csr_write(CSR_PMPADDR3, c910_regs.pmpaddr3);
		csr_write(CSR_PMPADDR4, c910_regs.pmpaddr4);
		csr_write(CSR_PMPADDR5, c910_regs.pmpaddr5);
		csr_write(CSR_PMPADDR6, c910_regs.pmpaddr6);
		csr_write(CSR_PMPADDR7, c910_regs.pmpaddr7);
		csr_write(CSR_PMPCFG0, c910_regs.pmpcfg0);

		csr_write(CSR_MCOR, c910_regs.mcor);
		csr_write(CSR_MHCR, c910_regs.mhcr);
		csr_write(CSR_MHINT, c910_regs.mhint);
		csr_write(CSR_MXSTATUS, c910_regs.mxstatus);
	}

	return 0;
}

static int c910_final_init(bool cold_boot)
{
	c910_delegate_traps();

	return 0;
}

static int try_uart_port(void)
{
	unsigned int reg, port_num;

	reg = readl((unsigned int *)(SUNXI_CCU_BASE + SUNXI_UART_BGR_REG));

	for (port_num = 0; port_num <= SUNXI_UART_MAX; port_num++) {
		if (reg & (1 << port_num)) {
			return port_num;
			break;
		}
	}
	return -1;
}

static int sunxi_console_init(void)
{
	unsigned int port_num, uart_base;

	port_num = try_uart_port();
	uart_base = SUNXI_UART_BASE + port_num * SUNXI_UART_ADDR_OFFSET;

	return sunxi_uart_init(uart_base);
}

static int c910_irqchip_init(bool cold_boot)
{
	/* Delegate plic enable into S-mode */
	writel(C910_PLIC_DELEG_ENABLE,
		(void *)c910_regs.plic_base_addr + C910_PLIC_DELEG_OFFSET);

	return 0;
}

static int c910_ipi_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = clint_cold_ipi_init(c910_regs.clint_base_addr, C910_HART_COUNT);
		if (rc)
			return rc;
	}

	return clint_warm_ipi_init();
}

static int c910_timer_init(bool cold_boot)
{
	int ret;

	if (cold_boot) {
		ret = clint_cold_timer_init(c910_regs.clint_base_addr,
					C910_HART_COUNT, FALSE);
		if (ret)
			return ret;
	}

	return clint_warm_timer_init();
}

static int c910_system_shutdown(u32 type)
{
	/*TODO:power down something*/
	while(1);
	return 0;
}

static int sunxi_system_reboot(u32 type)
{

	sbi_printf("sbi reboot\n");
	unsigned int value = 0;
	void *reg = NULL;

	/* config reset whole system */
	value = (0x1 << SUNXI_WDOG_CFG_CONFIG_OFFSET) |
		       (SUNXI_WDOG_KEY1 << SUNXI_WDOG_CFG_KEY_OFFSET);
	reg = (void *)(SUNXI_WDOG_BASE + SUNXI_WDOG_CFG_REG);
	writel(value,reg);

	/* enable wdog */
	value = (0x1 << SUNXI_WDOG_MODE_EN_OFFSET) |
		       (SUNXI_WDOG_KEY1 << SUNXI_WDOG_CFG_KEY_OFFSET);
	reg = (void *)(SUNXI_WDOG_BASE + SUNXI_WDOG_MODE_REG);
	writel(value, reg);
	return 0;
}

void sbi_set_pmu()
{
	unsigned long interrupts;

	interrupts = csr_read(CSR_MIDELEG) | (1 << 17);
	csr_write(CSR_MIDELEG, interrupts);

	/* CSR_MCOUNTEREN has already been set in mstatus_init() */
	csr_write(CSR_MCOUNTERWEN, 0xffffffff);
	csr_write(CSR_MHPMEVENT3, 1);
	csr_write(CSR_MHPMEVENT4, 2);
	csr_write(CSR_MHPMEVENT5, 3);
	csr_write(CSR_MHPMEVENT6, 4);
	csr_write(CSR_MHPMEVENT7, 5);
	csr_write(CSR_MHPMEVENT8, 6);
	csr_write(CSR_MHPMEVENT9, 7);
	csr_write(CSR_MHPMEVENT10, 8);
	csr_write(CSR_MHPMEVENT11, 9);
	csr_write(CSR_MHPMEVENT12, 10);
	csr_write(CSR_MHPMEVENT13, 11);
	csr_write(CSR_MHPMEVENT14, 12);
	csr_write(CSR_MHPMEVENT15, 13);
	csr_write(CSR_MHPMEVENT16, 14);
	csr_write(CSR_MHPMEVENT17, 15);
	csr_write(CSR_MHPMEVENT18, 16);
	csr_write(CSR_MHPMEVENT19, 17);
	csr_write(CSR_MHPMEVENT20, 18);
	csr_write(CSR_MHPMEVENT21, 19);
	csr_write(CSR_MHPMEVENT22, 20);
	csr_write(CSR_MHPMEVENT23, 21);
	csr_write(CSR_MHPMEVENT24, 22);
	csr_write(CSR_MHPMEVENT25, 23);
	csr_write(CSR_MHPMEVENT26, 24);
	csr_write(CSR_MHPMEVENT27, 25);
	csr_write(CSR_MHPMEVENT28, 26);
}

extern void sbi_system_suspend(int state);
extern void sbi_system_set_wakeup(unsigned long irq, unsigned long on);

void sbi_boot_other_core(int hartid)
{
	csr_write(CSR_MRVBR, FW_TEXT_START);
	csr_write(CSR_MRMR, csr_read(CSR_MRMR) | (1 << hartid));
}

static int c910_vendor_ext_provider(long extid, long funcid,
				unsigned long *args,
				unsigned long *out_value,
				struct sbi_trap_info *out_trap)
{
	switch (extid) {
	case SBI_EXT_VENDOR_C910_BOOT_OTHER_CORE:
		sbi_boot_other_core((int)args[0]);
		break;
	case SBI_EXT_VENDOR_C910_SET_PMU:
		sbi_set_pmu();
		break;
	case SBI_EXT_VENDOR_C910_SYSPEND:
		sbi_system_suspend(args[0]);
		break;
	case SBI_SET_WAKEUP_TIMER:
		sbi_set_wakeup_src_timer((unsigned int)args[0]);
		break;
	case SBI_SET_DEBUG_LEVEL:
		break;
	case SBI_SET_DEBUG_DRAM_CRC_PARAS:
		sbi_set_dram_crc_paras(args[0], args[1], args[2]);
		break;
	case SBI_SET_UART_BAUDRATE:
		break;
	case SBI_EXT_VENDOR_C910_WAKEUP:
		sbi_system_set_wakeup(args[0], args[1]);
		break;

	default:
		sbi_printf("Unsupported private sbi call: %ld\n", extid);
		asm volatile ("ebreak");
	}
	return 0;
}

/* Get number of PMP regions for given HART. */
static u32 sunxi_pmp_region_count(u32 hartid)
{
	return 4;
}

static unsigned long log2roundup(unsigned long x)
{
	unsigned long ret = 0;

	while (ret < __riscv_xlen) {
		if (x <= (1UL << ret))
			break;
		ret++;
	}

	return ret;
}
/*
 * Get PMP regions details (namely: protection, base address, and size) for
 * a given HART.
 */
static int sunxi_pmp_region_info(u32 hartid, u32 index, ulong *prot,
				 ulong *addr, ulong *log2size)
{
	int ret = 0;

	switch (index) {
	case 0:
		*prot	  = PMP_R | PMP_W | PMP_X;
		*addr	  = 0x40000000;
		*log2size = log2roundup(0x40000000);
		break;
	case 1:
		*prot	  = PMP_R | PMP_W | PMP_X;
		*addr	  = 0x80000000;
		*log2size = log2roundup(0x40000000);
		break;
	case 2:
		*prot	  = PMP_R | PMP_W | PMP_X;
		*addr	  = 0x20000;
		*log2size = log2roundup(0x8000);
		break;
	case 3:
		*prot	  = PMP_R | PMP_W;
		*addr	  = 0x0;
		*log2size = log2roundup(0x40000000);
		break;
	default:
		ret = -1;
		break;
	};

	return ret;
}

const struct sbi_platform_operations platform_ops = {
	.early_init          = c910_early_init,
	.final_init          = c910_final_init,

	.pmp_region_count    = sunxi_pmp_region_count,
	.pmp_region_info     = sunxi_pmp_region_info,

	.console_putc        = sunxi_uart_putc,
	.console_getc        = sunxi_uart_getc,
	.console_init        = sunxi_console_init,

	.irqchip_init        = c910_irqchip_init,

	.ipi_init            = c910_ipi_init,
	.ipi_send            = clint_ipi_send,
	.ipi_clear           = clint_ipi_clear,

	.timer_init          = c910_timer_init,
	.timer_event_start   = clint_timer_event_start,

	.system_reboot      = sunxi_system_reboot,
	.system_shutdown     = c910_system_shutdown,

	.vendor_ext_provider = c910_vendor_ext_provider,
};

const struct sbi_platform platform = {
	.opensbi_version     = OPENSBI_VERSION,
	.platform_version    = SBI_PLATFORM_VERSION(0x0, 0x01),
	.name                = "T-HEAD Xuantie Platform",
	.features            = SBI_THEAD_FEATURES,
	.hart_count          = C910_HART_COUNT,
	.hart_stack_size     = C910_HART_STACK_SIZE,
	.disabled_hart_mask  = 0,
	.platform_ops_addr   = (unsigned long)&platform_ops
};
