// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/main.c
 *
 * Copyright (c) 2018 Allwinner.
 * 2018-09-14 Written by fanqinghua (fanqinghua@allwinnertech.com).
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "type.h"
#include "standby.h"
#include "main.h"
#include "head.h"
#include "power.h"
#include "driver/twi/standby_twi.h"
#include "clk.h"
#include "delay.h"
#include "dram.h"
#include "head.h"
#include "uart.h"
#include "sunxi_cpu.h"

#define SUNXI_UART_ADDR            0x02500000
#define HOTPLUG_FLAG_REG                  0x70005DC
#define HOTPLUG_FLAG_VAL                  0xFA50392F
#define HOTPLUG_SOFTENTRY_REG             0x70005E0

extern void sbi_system_resume(unsigned long addr);
extern void sbi_suspend_finish();
sram_head_t *para = (sram_head_t *)SRAM_BASE;
__dram_para_t *dram_para = (void *)0x20038;
unsigned long SBI_GPRS[SBI_GPR_MAX];

void sbi_ppu_set_entry(unsigned long addr)
{
	unsigned long entry = addr;
#ifdef SUNXI_RESET_ENTRY
	writel((entry & 0xFFFFFFFF), (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_RESET_ENTRY_LOW));
	writel((entry >> 32), (void *)(SUNXI_RISCV_CFG_BASE + SUNXI_RESET_ENTRY_LOW));
#else
	writel(HOTPLUG_FLAG_VAL, (void *)HOTPLUG_FLAG_REG);
	writel((entry & 0xFFFFFFFF), (void *)HOTPLUG_SOFTENTRY_REG);

#endif
}

static __attribute__((noinline)) int _standby_main(void)
{
	char *tmp_ptr = (char *)&__bss_start;

	/* clear bss segment */
	do {
		*tmp_ptr++ = 0;
	} while (tmp_ptr <= (char *)&__bss_end);

	sunxi_uart_init(SUNXI_UART_ADDR);

	save_mem_status(STANDBY_START | 0X01);

	save_mem_status(STANDBY_START | 0X02);

	/* calc dram checksum */
	if (para->crc_enable) {
		para->crc_before = dram_crc(para);
	}
	save_mem_status(STANDBY_START | para->crc_before);
	set_dram_crc_paras(para->crc_enable, para->crc_start, para->crc_len);

	/* cpu2hosc(); */
	udelay(10);

	/* enable dram enter self-refresh */
	dram_enter_selfresh(dram_para);

	system_suspend(para);

	if (!para->time_to_wakeup_ms) {
		sbi_ppu_set_entry((long)sbi_system_resume);
		sbi_suspend_finish(SBI_GPRS);
	} else {
		mdelay(para->time_to_wakeup_ms);
	}

	system_restore(para);

	dram_exit_selfresh(dram_para);

	if (para->crc_enable) {
		para->crc_after = dram_crc(para);
		if (para->crc_after != para->crc_before) {
			pr_info("dram crc error\n");
			while (1);
		} else {
			pr_info("dram crc good\n");
		}
	}

	/* cpu2pll(); */
	/* udelay(10); */

	return 0;
}

#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)
#define SP_ADDRESS 0x27ff0


#define nop()		__asm__ __volatile__ ("nop")

#define RISCV_FENCE(p, s) \
	__asm__ __volatile__ ("fence " #p "," #s : : : "memory")

/* These barriers need to enforce ordering on both devices or memory. */
#define mb()		RISCV_FENCE(iorw,iorw)
#define rmb()		RISCV_FENCE(ir,ir)
#define wmb()		RISCV_FENCE(ow,ow)

/* These barriers do not need to enforce ordering on devices, just memory. */
#define __smp_mb()	RISCV_FENCE(rw,rw)
#define __smp_rmb()	RISCV_FENCE(r,r)
#define __smp_wmb()	RISCV_FENCE(w,w)

uint64_t __attribute__((section(".data"))) savebp;
int standby_main(void)
{
	asm volatile ("sd sp, savebp, t0");
	asm volatile ("li sp, " __stringify(SP_ADDRESS));
	asm volatile ("fence.i" ::: "memory");

	_standby_main();

	asm volatile ("ld sp, savebp");
	asm volatile ("fence.i" ::: "memory");
}
