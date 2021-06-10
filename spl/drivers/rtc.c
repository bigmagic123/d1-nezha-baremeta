
/*
 * Sunxi RTC data area ops
 *
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <arch/rtc.h>

#define CRASHDUMP_RESET_FLAG                (0x5AA55AA5)
#define CRASHDUMP_RESET_READY               (0x5AA55AA6)
#define CRASHDUMP_REFRESH_READY             (0x5AA55AA7)
#define EFEX_FLAG                           (0x5AA5A55A)
#define RTC_INDEX  2


void rtc_write_data(int index, u32 val)
{
	void __iomem *rtc_base = sunxi_get_iobase(SUNXI_RTC_DATA_BASE);
	writel(val, rtc_base + index * 4);
}

u32 rtc_read_data(int index)
{
	void __iomem *rtc_base = sunxi_get_iobase(SUNXI_RTC_DATA_BASE);
	return readl(rtc_base + index * 4);
}

void rtc_set_fel_flag(void)
{
	do {
		rtc_write_data(RTC_INDEX, EFEX_FLAG);
		data_sync_barrier();
	} while (rtc_read_data(RTC_INDEX) != EFEX_FLAG);
}

u32 rtc_probe_fel_flag(void)
{
	u32 i , reg_value;

	for (i=0; i<=5; i++) {
		reg_value = rtc_read_data(i);
		if (reg_value)
			printf("rtc[%d] value = 0x%x\n", i, reg_value);
	}

	reg_value = rtc_read_data(RTC_INDEX);
	if (reg_value == EFEX_FLAG) {
		printf("eraly jump fel\n");
		return 1;
	} else if (reg_value == CRASHDUMP_RESET_FLAG){
		rtc_write_data(RTC_INDEX, CRASHDUMP_RESET_READY);
		do {
			mdelay(150);
			reg_value = rtc_read_data(RTC_INDEX);
		}
		while (reg_value != CRASHDUMP_REFRESH_READY);
		printf("carshdump mode , jump fel\n");
		return 1;
	}
	return 0;
}

void rtc_clear_fel_flag(void)
{
	do {
		rtc_write_data(RTC_INDEX, 0);
		data_sync_barrier();
	} while (rtc_read_data(RTC_INDEX) != 0);
}

void rtc_set_hash_entry(phys_addr_t entry)
{
	do {
		rtc_write_data(RTC_INDEX, GET_LO32(entry));
		data_sync_barrier();
	} while (rtc_read_data(RTC_INDEX) != entry);
}
