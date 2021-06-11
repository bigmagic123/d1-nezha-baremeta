/*
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _C910_PLATFORM_H_
#define _C910_PLATFORM_H_

#define C910_HART_COUNT   1
#define C910_HART_STACK_SIZE   8192

#define SBI_THEAD_FEATURES	\
	(SBI_PLATFORM_HAS_PMP | \
	 SBI_PLATFORM_HAS_SCOUNTEREN | \
	 SBI_PLATFORM_HAS_MCOUNTEREN | \
	 SBI_PLATFORM_HAS_MFAULTS_DELEGATION)

#define CSR_MCOR         0x7c2
#define CSR_MHCR         0x7c1
#define CSR_MCCR2        0x7c3
#define CSR_MHINT        0x7c5
#define CSR_MXSTATUS     0x7c0
#define CSR_PLIC_BASE    0xfc1
#define CSR_MRMR         0x7c6
#define CSR_MRVBR        0x7c7
#define CSR_MCOUNTERWEN  0x7c9

#define CSR_SMIR         0x9C0
#define CSR_SMEL         0x9C1
#define CSR_SMEH         0x9C2
#define CSR_SMCIR        0x9C3


#define SBI_EXT_VENDOR_C910_SET_PMU            0x09000001
#define SBI_EXT_VENDOR_C910_BOOT_OTHER_CORE    0x09000003
#define SBI_EXT_VENDOR_C910_SYSPEND            0x09000007
#define SBI_EXT_VENDOR_C910_WAKEUP             0x09000008

#define C910_PLIC_CLINT_OFFSET            0x04000000  /* 64M */
#define C910_PLIC_DELEG_OFFSET            0x001ffffc
#define C910_PLIC_H0_STH_OFFSET           0x0201000
#define C910_PLIC_PRI_OFFSET              0x04
#define C910_PLIC_PEND_OFFSET             0x1000
#define C910_PLIC_SIE_OFFSET              0x2080
#define C910_PLIC_DELEG_ENABLE            0x1

#define SUNXI_UART_BASE                   0x02500000
#define SUNXI_UART_ADDR_OFFSET            0x400
#define SUNXI_UART_MAX	                  5


#define SUNXI_PPU_BASE                    0X7001000
#define SUNXI_PD_ACTIVE_CTRL              0x2C

#define SUNXI_RISCV_CFG_BASE              0x6010000
#define SUNXI_RESET_ENTRY_LOW             0x4
#define SUNXI_RESET_ENTRY_HIGH            0x8
#define SUNXI_WAKEUP_EN_REG               0X20
#define SUNXI_WKAEUP_MASK0_REG            0X24
#define SUNXI_WKAEUP_MASK1_REG            0X28
#define SUNXI_WKAEUP_MASK2_REG            0X2c
#define SUNXI_WKAEUP_MASK3_REG            0X30
#define SUNXI_WKAEUP_MASK4_REG            0X34

#define SUNXI_CCU_BASE                    0x02001000
#define SUNXI_RISCV_CFG_BGR_REG           0xD0C

#define SUNXI_UART_BGR_REG	          0x90C


#define SUNXI_RPRCM_BASE                  0x7010000
#define SUNXI_RISCV_PPU_BUS_GATING        0x1AC


#define SUNXI_WDOG_BASE                  0x6011000
#define SUNXI_WDOG_CTRL_REG              0x10
#define SUNXI_WDOG_CTRL_KEY_OFFSET                  1
#define SUNXI_WDOG_RESTART_OFFSET                   0
#define SUNXI_WDOG_CFG_REG               0x14
#define SUNXI_WDOG_CFG_KEY_OFFSET                   16
#define SUNXI_WDOG_CFG_CONFIG_OFFSET                0
#define SUNXI_WDOG_MODE_REG              0x18
#define SUNXI_WDOG_MODE_KEY_OFFSET                  16
#define SUNXI_WDOG_MODE_INTV_OFFSET                 4
#define SUNXI_WDOG_MODE_EN_OFFSET                   0
#define SUNXI_WDOG_KEY0                             0xA57
#define SUNXI_WDOG_KEY1                             0x16AA

#define HOTPLUG_FLAG_REG                  0x70005DC
#define HOTPLUG_FLAG_VAL                  0xFA50392F
#define HOTPLUG_SOFTENTRY_REG             0x70005E0

#define SUNXI_UART_ADDR            0x02500000
#define SUNXI_SRAM_ADDR            0x00020000

struct c910_regs_struct {
	u64 pmpaddr0;
	u64 pmpaddr1;
	u64 pmpaddr2;
	u64 pmpaddr3;
	u64 pmpaddr4;
	u64 pmpaddr5;
	u64 pmpaddr6;
	u64 pmpaddr7;
	u64 pmpcfg0;
	u64 mcor;
	u64 mhcr;
	u64 mccr2;
	u64 mhint;
	u64 mxstatus;
	u64 plic_base_addr;
	u64 clint_base_addr;
};
#endif /* _C910_PLATFORM_H_ */
