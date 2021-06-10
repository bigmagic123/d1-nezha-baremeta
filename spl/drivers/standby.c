/*
 *
 * (C) Copyright 2019
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * frank <frank@allwinnertech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <arch/rtc.h>
#include <arch/watchdog.h>
#include <arch/clock.h>

#define DRAM_CRC_MAGIC			(0x76543210)

typedef struct pm_dram_para {
	unsigned int selfresh_flag;
	unsigned int crc_en;
	unsigned int crc_start;
	unsigned int crc_len;
} pm_dram_para_t;

static pm_dram_para_t soc_dram_state;
static u32 before_crc;
static u32 after_crc;

static u32 standby_dram_crc_enable(pm_dram_para_t *pdram_state)
{
	return pdram_state->crc_en;
}

static u32 standby_dram_crc(pm_dram_para_t *pdram_state)
{
	u32 *pdata;
	u32 crc = 0;

	pdata = (u32 *)(pdram_state->crc_start);
	printf("src:0x%x\n", (unsigned int)pdata);
	printf("len addr = 0x%x, len:0x%x\n",
			(unsigned int) (&(pdram_state->crc_len)),
			pdram_state->crc_len);
	while (pdata < (u32 *)(pdram_state->crc_start +
				pdram_state->crc_len)) {
		crc += *pdata;
		pdata++;
	}
	printf("crc finish...\n");
	return crc;
}

static int probe_super_standby_flag(void)
{
	uint reg_value = 0;
	int standby_flag = 0;

	reg_value = readl(RTC_STANDBY_FLAG_REG);
	standby_flag = (reg_value & ~(0xfffe0000)) >> 16;
	printf("rtc standby flag is 0x%x, super standby flag is 0x%x\n",
			reg_value, standby_flag);
	writel(0, RTC_STANDBY_FLAG_REG);

	return standby_flag;
}

int super_standby_mode(void)
{
	uint reg_value = 0;
	int standby_flag = 0;

	reg_value = readl(RTC_STANDBY_FLAG_REG);
	standby_flag = (reg_value & ~(0xfffe0000)) >> 16;

	return standby_flag;
}

void handler_super_standby(void)
{
	if (probe_super_standby_flag()) {

		/* high 28 bits for magic number, low 4 bits for enable */
		if ((rtc_read_data(CRC_EN) & (~0xf)) == DRAM_CRC_MAGIC) {
			soc_dram_state.crc_en = rtc_read_data(CRC_EN) & 0xf;
			soc_dram_state.crc_start = rtc_read_data(CRC_START);
			soc_dram_state.crc_len = rtc_read_data(CRC_LEN);
		}

		if (standby_dram_crc_enable(&soc_dram_state)) {
			before_crc = rtc_read_data(CRC_VALUE_BEFORE);
			after_crc = standby_dram_crc(&soc_dram_state);
			printf("before_crc = 0x%x, after_crc = 0x%x\n",
					before_crc, after_crc);
			if (before_crc != after_crc) {
				printf("dram crc error ...\n");
				wdt_start(0);
				asm ("b .");
			}
		}
		/*
		 * /dram_enable_all_master(); Not implemented on fpga
		 */
		printf("find standby flag, jump to addr 0x%x\n",
			readl(RTC_STANDBY_SOFT_ENTRY_REG));
		/* FIX ME: need 500ms ?
		 *__msdelay(500);
		 */
#ifdef CFG_SUNXI_SBOOT
	int sunxi_smc_resume(int secure_entry);
	/* Borrowing the standby jmp address
	 * to calculate the address of monitor or optee
	 */
	sunxi_smc_resume(readl(RTC_STANDBY_SOFT_ENTRY_REG) & 0xfff00000);
#endif
		udelay(20);
#ifdef CONFIG_MONITOR
		boot0_jmp_monitor(readl(RTC_STANDBY_SOFT_ENTRY_REG));
#else
		boot0_jmp(readl(RTC_STANDBY_SOFT_ENTRY_REG));
#endif
	}
}
