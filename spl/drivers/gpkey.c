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

int sunxi_clock_init_gpadc(void);

int sunxi_gpadc_init(void)
{
	uint reg_val = 0;

	sunxi_clock_init_gpadc();

	/*ADC sample ferquency divider*ferquency=CLK_IN/(n+1), 24000000/(0x5dbf+1)=1000Hz
	* ADC acquire time
	* time=CLK_IN/(n+1) ,24000000/(0x2f+1)=500000Hz
	*/
	reg_val = readl(GP_SR_CON);
	reg_val &= ~(0xffff << 16);
	reg_val |= (0x5dbf << 16);
	writel(reg_val, GP_SR_CON);


	/*choose continue work mode*/
	reg_val = readl(GP_CTRL);
	reg_val &= ~(1<<18);
	reg_val |= (1 << 19);
	writel(reg_val, GP_CTRL);

	/*enable ADC*/
	reg_val = readl(GP_CTRL);
	reg_val |= (1 << 16);
	writel(reg_val, GP_CTRL);

	/* disable all key irq */
	writel(0, GP_DATA_INTC);
	writel(1, GP_DATA_INTS);


	return 0;
}


int sunxi_read_gpadc(int channel)
{

	u32 ints;
	u32 adc_val = 0;

	/*choose channel */
	setbits_le32(GP_CS_EN, 0x01 << channel);
	udelay(1500);

	ints = readl(GP_DATA_INTS);

	/* clear the pending status */
	writel((ints & (0x1 << channel)), GP_DATA_INTS);
	//writel((ints & 0x7), GP_DATA_INTS);

	/* if there is already data pending,read it .
	 * gpadc_data = adc_val/Vref*4095, and Vref=1.8v.
	 * adc_val should be 0~1.8v.
	 */
	if (ints & (GPADC0_DATA_PENDING << channel)) {
		adc_val = readl(GP_CH0_DATA + channel*4);
		//key = adc_val > 16 ? 0 : (readl(GP_CH0_DATA + channel*4)*63/4095);
	}

	return adc_val;
}

#define ADC_NUM 3
u32 get_sys_ticks(void);
int sunxi_read_gpadc_vol(int channel)
{
	int i, sum, temp_gpadc[ADC_NUM];
	int old_time = get_sys_ticks();
	while (1) {
		sum = 0;
		for (i = 0; i < ADC_NUM; i++) {
			temp_gpadc[i] = sunxi_read_gpadc(channel)*1800/4095;
			sum += temp_gpadc[i];
		/* printf("sum:%d\ttemp_gpadc[%d]:%d\n", sum, i, temp_gpadc[i]); */
		}
		/* printf("sum:%d\ttemp_gpadc[0]:%d\n", sum/ADC_NUM, temp_gpadc[0]); */
		if (abs(sum/ADC_NUM - temp_gpadc[0]) < 5) {
			break;
		} else if (get_sys_ticks() - old_time >= 2000) {
			printf("get adc time out!!!\n");
			break;
		}
	}
	return sum/ADC_NUM;
}

int sunxi_read_gpadc_key(int channel)
{
	int adc_val = sunxi_read_gpadc(channel);
	return adc_val > 16 ? 0 : adc_val*63/4095;
}
