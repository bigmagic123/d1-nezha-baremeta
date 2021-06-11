// SPDX-License-Identifier: BSD-2-Clause
/*
 * clk_standby.c
 *
 *  Created on: 2021年2月5日
 *      Author: caoyunlong
 */
#include "clk-plat.h"
#include "type.h"
#include "clk.h"
#include "delay.h"
#include "clk-sun20iw1p1.h"

typedef struct BUS_PAMA
{
	void *busAddr;
	u8 bus_clk_src;
	u8 N;
	u8 M;
	u8 flag; //check the first write is or not default;
}rBUS_PAMA;

static rBUS_PAMA __attribute__((section(".data"))) bus_restore[7];

enum pll_state
{
	pll_disable	 = 0,
	pll_enable  = 1
};

enum pmu_state
{
	susper_standby	 = 0,
	normal_standby  = 1,
	usb_standby	 = 2,
	elink_standby= 3
};


enum BUS_CLOCK_SOURCE
{
	osc_24M	 = 0,
	rtc_32k  = 1,
	rc16m	 = 2,
	pll_peri0= 3,
	dft_src  = 4
};

static void bus_clock_source(enum BUS_CLOCK_SOURCE mode)
{
	u32 reg_val, bus_tick;

	bus_restore[0].busAddr = (void *)CCMU_CPUX_C0_AXI_CFG_REG;
	bus_restore[1].busAddr = (void *)RISCV_CLK_CFG_REG;
	bus_restore[2].busAddr = (void *)CCMU_PSI_CFG_REG;
	bus_restore[3].busAddr = (void *)CCMU_APB1_CFG_REG;
	bus_restore[4].busAddr = (void *)CCMU_APB2_CFG_REG;
	bus_restore[5].busAddr = (void *)CPUS_CLK_CFG_REG;
	bus_restore[6].busAddr = (void *)APBS1_CFG_REG;

	for(bus_tick=0;bus_tick<(sizeof(bus_restore)/sizeof(bus_restore[0]));bus_tick++){
		reg_val = readl(bus_restore[bus_tick].busAddr);
		if((mode == dft_src)&&(bus_restore[bus_tick].flag!=0)){
			reg_val |=  bus_restore[bus_tick].N<<8;
			reg_val |=  bus_restore[bus_tick].M<<0;
			writel(reg_val,bus_restore[bus_tick].busAddr);
			reg_val |=  bus_restore[bus_tick].bus_clk_src<<24;
			writel(reg_val,bus_restore[bus_tick].busAddr);
			bus_restore[bus_tick].flag = 0;
		}
		else{
			if(bus_restore[bus_tick].flag == 0){
				bus_restore[bus_tick].bus_clk_src = (reg_val>>24)&0x7;
				bus_restore[bus_tick].N = (reg_val>>8)&0x3;
				bus_restore[bus_tick].M = reg_val&0x1f;
				bus_restore[bus_tick].flag = 1;
			}
			else
				bus_restore[bus_tick].flag = 1;
			reg_val &= ~(0x7<<24);
			reg_val |= (mode<<24);
			writel(reg_val,bus_restore[bus_tick].busAddr);
			reg_val &= ~(0x3ff<<0);
			writel(reg_val,bus_restore[bus_tick].busAddr);
		}
		udelay(20);
		/* printk("addr:0x%x------value:0x%x\r\n",bus_restore[bus_tick].busAddr,readl(bus_restore[bus_tick].busAddr)); */
	}
}

static void all_pll_set(enum pll_state sta)
{
	u32 reg_val,i;
	u32 pll_list[] = {
			CCMU_PLL_CPUX_C0_CTRL_REG,
			CCMU_PLL_DDR0_CTRL_REG,
			CCMU_PLL_PERI0_CTRL_REG,
			CCMU_PLL_VIDEO0_CTRL_REG,
			CCMU_PLL_VIDEO1_CTRL_REG,
			CCMU_PLL_VE_CTRL_REG,
			CCMU_PLL_AUDIO_CTRL_REG,
			CCMU_PLL_AUDIO1_CTRL_REG,
	};
	for(i=0;i<(sizeof(pll_list)/sizeof(pll_list[0]));i++)
	{
		reg_val = readl((void *)(long)pll_list[i]);
		reg_val &= (~(1<<31));
		reg_val |= (sta<<31);
		reg_val |= (sta<<29);
		writel(reg_val,(void *)(long)pll_list[i]);
	}
}

static void dcxo_disable(void)
{
	u32 reg_val;
	reg_val = readl(RTC_XO_CTRL);
	reg_val &= ~(0x1<<1);
	reg_val |= (0x1<<31);
	writel(reg_val,(void *)RTC_XO_CTRL);
}
static void dcxo_enable(void)
{
	u32 reg_val;
	reg_val = readl(RTC_XO_CTRL);
	reg_val |= (0x1<<1);
	reg_val &= ~(0x1<<31);
	writel(reg_val,(void *)RTC_XO_CTRL);
}
static void rtc32k_disable(void)
{
	u32 reg_val;
	reg_val = readl(RTC_LOSC_CTRL);
	reg_val &= ~(0x1<<0);
	reg_val |= (0x16aa<<16);
	writel(reg_val,(void *)RTC_LOSC_CTRL);
	writel(reg_val,(void *)RTC_LOSC_CTRL);
	udelay(10);
//	udelay(1);
	reg_val = readl(RTC_LOSC_CTRL);
	reg_val &= ~(0x1<<4);
	reg_val |= (0x16aa<<16);
	writel(reg_val, (void *)RTC_LOSC_CTRL);
	writel(reg_val,(void *)RTC_LOSC_CTRL);
	udelay(10);
//	udelay(1);

	reg_val = readl(RTC_LOSC_OUT_GATING);
	reg_val &= ~(0x1<<0);
	writel(reg_val,(void *)RTC_LOSC_OUT_GATING);
}
static void rtc32k_enable(void)
{
	u32 reg_val;
	reg_val = readl(RTC_LOSC_CTRL);
	reg_val |= (0x1<<4);
	reg_val |= (0xa7<<16);
	writel(reg_val,(void *)RTC_LOSC_CTRL);
	writel(reg_val,(void *)RTC_LOSC_CTRL);
	udelay(10);
	reg_val = readl(RTC_LOSC_CTRL);
	reg_val |= (0x1<<0);
	reg_val |= (0x16aa<<16);
	writel(reg_val,(void *)RTC_LOSC_CTRL);
	writel(reg_val,(void *)RTC_LOSC_CTRL);
	udelay(10);
	reg_val = readl(RTC_LOSC_OUT_GATING);
	reg_val |= (0x1<<0);
	writel(reg_val,(void *)RTC_LOSC_OUT_GATING);
}

static void pll_ldo_disable(void){

	u32 reg_val;
	reg_val = readl(PLL_CTRL_REG1);
	reg_val &= ~(0x1<<0);
	reg_val |= (0xa7<<24);
	writel(reg_val,(void *)PLL_CTRL_REG1);
	writel(reg_val,(void *)PLL_CTRL_REG1);
}
static void pll_ldo_enable(void){

	u32 reg_val;
	reg_val = readl(PLL_CTRL_REG1);
	reg_val |= (0x1<<0);
	reg_val |= (0xa7<<24);
	writel(reg_val,(void *)PLL_CTRL_REG1);
	writel(reg_val,(void *)PLL_CTRL_REG1);
}

void system_suspend(sram_head_t *para)
{
	bus_clock_source(osc_24M);
	all_pll_set(pll_disable);
	rtc32k_disable();
	pll_ldo_disable();
//	dcxo_disable();//CPUIDLE与24M有关，不能关闭24M时钟
}

void system_restore(sram_head_t *para)
{
//	dcxo_enable();
//	udelay(20);
	pll_ldo_enable();
	mdelay(4);
	rtc32k_enable();
	mdelay(4);
	all_pll_set(pll_enable);
	mdelay(2);
	bus_clock_source(dft_src);
	mdelay(2);
}

