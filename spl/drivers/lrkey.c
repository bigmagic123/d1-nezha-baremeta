/*
 * (C) Copyright 2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * zhouhuacai <zhouhuacai@allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <arch/physical_key.h>
#include <arch/clock.h>
#include <arch/axp.h>

__attribute__((section(".data")))
static uint32_t keyen_flag = 1;

__weak int sunxi_clock_init_key(void)
{
	return 0;
}

int sunxi_key_init(void)
{
	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;
	uint reg_val = 0;

	sunxi_clock_init_key();

	reg_val = sunxi_key_base->ctrl;
	reg_val &= ~((7 << 1) | (0xffU << 24));
	reg_val |=  LRADC_HOLD_EN;
	reg_val |=  LRADC_EN;
	sunxi_key_base->ctrl = reg_val;

	/* disable all key irq */
	sunxi_key_base->intc = 0;
	sunxi_key_base->ints = 0x1f1f;

	return 0;
}

int sunxi_key_read(void)
{
	u32 ints;
	int key = -1;
	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_LRADC_BASE;

	if (!keyen_flag) {
		return -1;
	}

	ints = sunxi_key_base->ints;
	/* clear the pending data */
	sunxi_key_base->ints |= (ints & 0x1f);

	/* if there is already data pending,
	 read it */
	mdelay(10);
	if (ints & ADC0_KEYDOWN_PENDING) {
		if (ints & ADC0_DATA_PENDING) {
			key = sunxi_key_base->data0 & 0x3f;
		}
	} else if (ints & ADC0_DATA_PENDING) {
		key = sunxi_key_base->data0 & 0x3f;
	}

	if (key > 0) {
		printf("key pressed value=0x%x\n", key);
	}

	return key;
}

int check_update_key(u16 *key_input)
{
	int power_key_cnt = 0;
	u32 time_tick = get_sys_ticks();
	*key_input = sunxi_key_read();
	while (sunxi_key_read() > 0) {
		if (probe_power_key() > 0)
			power_key_cnt++;
		if (power_key_cnt >= 3)
			return -1;
		if ((get_sys_ticks() - time_tick) > 3000)
			break;
	}
	if ((!power_key_cnt) && (get_power_source() != AXP_BOOT_SOURCE_BUTTON))
		*key_input = 0;
	return 0;
}

