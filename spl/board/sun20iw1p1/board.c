/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
* SPDX-License-Identifier:	GPL-2.0+
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Some init for sunxi platform.
 *
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <private_boot0.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/efuse.h>
#include <arch/rtc.h>
#include <config.h>

#ifndef FPGA_PLATFORM
int sunxi_board_init(void)
{
	sunxi_board_pll_init();
	axp_init(0);
	/*
	 * When waking up, don't change the voltage, frequency,
	 * cpu is easy to hang. We restore it in atf.
	 */
		set_pll_voltage(CONFIG_SUNXI_CORE_VOL);
		set_sys_voltage(CONFIG_SUNXI_SYS_VOL);
	printf("board init ok\n");
	return 0;
}
#else
int sunxi_board_init(void)
{
	printf("FPGA board init ok\n");
	return 0;
}

#endif

uint8_t sunxi_board_late_init(void)
{
#ifdef CFG_SUNXI_KEY_PROVISION
void sunxi_key_provision(void);
	sunxi_key_provision();
#endif
#ifdef CFG_SUNXI_POWER
	if (get_pmu_exist() <= 0) {
#ifdef CFG_SUNXI_TWI
		 i2c_exit();
#endif
	}
#endif
	return 0;
}

int sunxi_deassert_arisc(void)
{
	printf("set arisc reset to de-assert state\n");
	{
		volatile unsigned int value;
		void *base = sunxi_get_iobase(SUNXI_RCPUCFG_BASE);

		value = readl(base + 0x0);
		value &= ~1;
		writel(value, base + 0x0);
		value = readl(base + 0x0);
		value |= 1;
		writel(value, base + 0x0);
	}

	return 0;
}

int sunxi_board_exit(void)
{
	return 0;
}

#ifdef CFG_SUNXI_KEY_PROVISION
#include <cache_align.h>
#include <arch/ce.h>
#include <arch/efuse.h>
#define ALIGNED_HUK_SIZE (ALIGN(SID_HUK_SIZE >> 3, CACHE_LINE_SIZE))
void sunxi_key_provision(void)
{
	uint32_t trng_buf[ALIGNED_HUK_SIZE >> 2];
	int i;
	uint32_t write_protect;
	write_protect = sid_sram_read(EFUSE_WRITE_PROTECT);

	/*ndump((u8*)0x3006200,512);*/

	if (write_protect & (1 << EFUSE_HUK_PROTECT_OFFSET)) {
		/*already burned, do nothing*/
		//printf("huk already burned\n");
		return;
	}
	for (i = 0; i < ALIGNED_HUK_SIZE; i += TRNG_BYTE_LEN) {
		sunxi_trng_gen((uint8_t *)&trng_buf[i >> 2], TRNG_BYTE_LEN);
	}

#if 0
	printf("gen trng huk:\n");
	ndump((u8 *)trng_buf, ALIGNED_HUK_SIZE);
	printf("burn efuse:\n");
#endif

	for (i = 0; i < (SID_HUK_SIZE >> 3); i += 4) {
		//printf("0x%x: 0x%x\n", EFUSE_HUK + i, trng_buf[i >> 2]);
		sid_program_key(EFUSE_HUK + i, trng_buf[i >> 2]);

		/*read again, load data into efuse sram*/
		sid_read_key(EFUSE_HUK + i);
	}

	write_protect |= (1 << EFUSE_HUK_PROTECT_OFFSET);
	sid_program_key(EFUSE_WRITE_PROTECT, write_protect);
	sid_read_key(EFUSE_WRITE_PROTECT);
#if 0
	printf("0x%x: 0x%x\n", EFUSE_WRITE_PROTECT, write_protect);
	ndump((u8 *)0x3006200, 512);
#endif
	return;
}
#endif
