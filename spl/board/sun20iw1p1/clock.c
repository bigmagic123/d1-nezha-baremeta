/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/efuse.h>


static void set_pll_cpux_axi(void)
{
	u32 reg_val;
	void __iomem *cpux_base = sunxi_get_iobase(CCMU_CPUX_AXI_CFG_REG);
	void __iomem *pll_base = sunxi_get_iobase(CCMU_PLL_CPUX_CTRL_REG);

	/* select CPUX  clock src: OSC24M,AXI divide ratio is 3, system apb clk ratio is 4 */
	writel((0 << 24) | (3 << 8) | (1 << 0), cpux_base);
	udelay(1);

	/* disable pll gating*/
	reg_val = readl(pll_base);
	reg_val &= ~(1 << 27);
	writel(reg_val, pll_base);

	reg_val = readl(pll_base);
	reg_val |= (0x1U << 30);
	writel(reg_val, pll_base);
	udelay(5);

	/* set default val: clk is 1008M  ,PLL_OUTPUT= 24M*N/( M*P)*/
	reg_val = readl(pll_base);
	reg_val &= ~((0x3 << 16) | (0xff << 8) | (0x3 << 0));
	reg_val |= (41 << 8);
	writel(reg_val, pll_base);

	/* lock enable */
	reg_val = readl(pll_base);
	reg_val |= (1 << 29);
	writel(reg_val, pll_base);

	/* enable pll */
	reg_val = readl(pll_base);
	reg_val |= (1 << 31);
	writel(reg_val, pll_base);

/*wait PLL_CPUX stable*/
#ifndef FPGA_PLATFORM
	while (!(readl(pll_base) & (0x1 << 28)))
		;
	udelay(20);
#endif
	/* enable pll gating*/
	reg_val = readl(pll_base);
	reg_val |= (1 << 27);
	writel(reg_val, pll_base);

	/* lock disable */
	reg_val = readl(pll_base);
	reg_val &= ~(1 << 29);
	writel(reg_val, pll_base);

	udelay(1);
	/*set and change cpu clk src to PLL_CPUX,  PLL_CPUX:AXI0 = 1008M:504M*/
	reg_val = readl(cpux_base);
	reg_val &= ~(0x07 << 24 | 0x3 << 8 | 0xf << 0);
	reg_val |= (0x05 << 24 | 0x1 << 8);
	writel(reg_val, cpux_base);
	udelay(1);
}

static void set_pll_periph0(void)
{
	u32 reg_val;
	void __iomem *pll_base = sunxi_get_iobase(CCMU_PLL_PERI0_CTRL_REG);

	if ((1U << 31) & readl(pll_base)) {
		/*fel has enable pll_periph0*/
		printf("periph0 has been enabled\n");
		return;
	}
	/*change  psi/ahb src to OSC24M before set pll6
	reg_val = readl(CCMU_PSI_AHB1_AHB2_CFG_REG);
	reg_val &= (~(0x3<<24));
	writel(reg_val,CCMU_PSI_AHB1_AHB2_CFG_REG) */;

	/* set default val*/
	writel(0x63 << 8, pll_base);

	/* lock enable */
	reg_val = readl(pll_base);
	reg_val |= (1 << 29);
	writel(reg_val, pll_base);

	/* enabe PLL: 600M(1X)  1200M(2x)*/
	reg_val = readl(pll_base);
	reg_val |= (1 << 31);
	writel(reg_val, pll_base);

#ifndef FPGA_PLATFORM
	while (!(readl(pll_base) & (0x1 << 28)))
		;
	udelay(20);
#endif
	/* lock disable */
	reg_val = readl(pll_base);
	reg_val &= (~(1 << 29));
	writel(reg_val, pll_base);
}

static void set_ahb(void)
{
	void __iomem *bus_base = sunxi_get_iobase(CCMU_PSI_AHB1_AHB2_CFG_REG);

	/* PLL6:AHB1:AHB2 = 600M:200M:200M */
	writel((2 << 0) | (0 << 8), bus_base);
	writel((0x03 << 24) | readl(bus_base), bus_base);
	udelay(1);
}

static void set_apb(void)
{
	void __iomem *bus_base = sunxi_get_iobase(CCMU_APB1_CFG_GREG);
	/*PLL6:APB1 = 600M:100M */
	writel((2 << 0) | (1 << 8), bus_base);
	writel((0x03 << 24) | readl(bus_base), bus_base);
	udelay(1);
}

static void set_pll_dma(void)
{
	void __iomem *dma_base = sunxi_get_iobase(CCMU_DMA_BGR_REG);

	/*dma reset*/
	writel(readl(dma_base) | (1 << 16), dma_base);
	udelay(20);
	/*gating clock for dma pass*/
	writel(readl(dma_base) | (1 << 0), dma_base);
}

static void set_pll_mbus(void)
{
	u32 reg_val;
	void __iomem *mbus_base = sunxi_get_iobase(CCMU_MBUS_CFG_REG);

	/*reset mbus domain*/
	reg_val = readl(mbus_base);
	reg_val |= (0x1 << 30);
	writel(reg_val, mbus_base);
	udelay(1);
}

static void set_ldo_analog(void)
{
	u32 soc_version = (readl(SUNXI_SID_BASE + 0x200) >> 22) & 0x3f;
	u32 audio_codec_bg_trim = (readl(SUNXI_SID_BASE + 0x228) >> 16) & 0xff;

	clrbits_le32(SUNXI_CCM_BASE + 0xA5C, 1 << (SUNXI_GATING_BIT));
	udelay(2);
	clrbits_le32(SUNXI_CCM_BASE + 0xA5C, 1 << (SUNXI_RST_BIT));
	udelay(2);
	/* deassert audio codec reset */
	setbits_le32(SUNXI_CCM_BASE + 0xA5C, 1 << (SUNXI_RST_BIT));
	/* open the clock for audio codec */
	setbits_le32(SUNXI_CCM_BASE + 0xA5C, 1 << (SUNXI_GATING_BIT));

	if (soc_version == 0b1010 || soc_version == 0) {
		setbits_le32(SUNXI_AUDIO_CODEC + 0x31C, 1 << 1);

		setbits_le32(SUNXI_AUDIO_CODEC + 0x348, 1 << 30);

	}
	if (!audio_codec_bg_trim) {
		clrsetbits_le32(SUNXI_AUDIO_CODEC + 0x348, 0xff, 0x19 << 0);
	} else {
		clrsetbits_le32(SUNXI_AUDIO_CODEC + 0x348, 0xff, audio_codec_bg_trim << 0);
	}

}

static void set_circuits_analog(void)
{
	/* calibration circuits analog enable */
	/* sunxi_clear_bit(RES_CAL_CTRL_REG, BIT(1)); */

	/* clrsetbits_le32(RES_CAL_CTRL_REG, 1 << 0, 1 << 0); */
}


static void set_platform_config(void)
{
	set_ldo_analog();

	set_circuits_analog();
}

static void set_modules_clock(void)
{
	u32 reg_val, i;
	unsigned long ccmu_pll_addr[] = {
				CCMU_PLL_PERI0_CTRL_REG,
				CCMU_PLL_VIDE00_CTRL_REG,
				CCMU_PLL_VIDE01_CTRL_REG,
				CCMU_PLL_VE_CTRL_REG,
				CCMU_PLL_AUDIO0_CTRL_REG,
				CCMU_PLL_AUDIO1_CTRL_REG,
				};

	for (i = 0; i < sizeof(ccmu_pll_addr)/sizeof(ccmu_pll_addr[0]); i++) {
		reg_val = readl((const volatile void __iomem *)ccmu_pll_addr[i]);
		if (!(reg_val & (1 << 31))) {
			writel(reg_val, (volatile void __iomem *)ccmu_pll_addr[i]);

			reg_val = readl((const volatile void __iomem *)ccmu_pll_addr[i]);
			writel(reg_val | (1 << 31) | (1 << 30), (volatile void __iomem *)ccmu_pll_addr[i]);
#ifndef FPGA_PLATFORM
			/* lock enable */
			reg_val = readl((const volatile void __iomem *)ccmu_pll_addr[i]);
			reg_val |= (1 << 29);
			writel(reg_val, (volatile void __iomem *)ccmu_pll_addr[i]);

			while (!(readl((const volatile void __iomem *)ccmu_pll_addr[i]) & (0x1 << 28)))
				;
			udelay(20);

			reg_val = readl((const volatile void __iomem *)ccmu_pll_addr[i]);
			reg_val &= ~(1 << 29);
			writel(reg_val, (volatile void __iomem *)ccmu_pll_addr[i]);
#endif
		}
	}
}

void sunxi_board_pll_init(void)
{
	printf("set pll start\n");
	set_platform_config();
	set_pll_cpux_axi();
	set_pll_periph0();
	set_ahb();
	set_apb();
	set_pll_dma();
	set_pll_mbus();
	set_modules_clock();
	printf("set pll end\n");
	return;
}

void sunxi_board_clock_reset(void)
{
	u32 reg_val;
	void __iomem *ahb_base = sunxi_get_iobase(CCMU_PSI_AHB1_AHB2_CFG_REG);
	void __iomem *apb_base = sunxi_get_iobase(CCMU_APB1_CFG_GREG);
	void __iomem *cpu_base = sunxi_get_iobase(CCMU_CPUX_AXI_CFG_REG);

	/*set ahb,apb to default, use OSC24M*/
	reg_val = readl(ahb_base);
	reg_val &= (~((0x3 << 24) | (0x3 << 8) | (0x3)));
	writel(reg_val, ahb_base);

	reg_val = readl(apb_base);
	reg_val &= (~((0x3 << 24) | (0x3 << 8) | (0x3)));
	writel(reg_val, apb_base);

	/*set cpux pll to default,use OSC24M*/
	writel(0x0301, cpu_base);
	return;
}

int sunxi_clock_init_key(void)
{
	uint reg_val = 0;
	void __iomem *gpadc_bgr_base = sunxi_get_iobase(CCMU_GPADC_BGR_REG);

	/* reset */
	reg_val = readl(gpadc_bgr_base);
	reg_val &= ~(1 << 16);
	writel(reg_val, gpadc_bgr_base);

	udelay(2);

	reg_val |= (1 << 16);
	writel(reg_val, gpadc_bgr_base);

	/* enable KEYADC gating */
	reg_val = readl(gpadc_bgr_base);
	reg_val |= (1 << 0);
	writel(reg_val, gpadc_bgr_base);

	return 0;
}

int sunxi_clock_exit_key(void)
{
	uint reg_val = 0;
	void __iomem *gpadc_bgr_base = sunxi_get_iobase(CCMU_GPADC_BGR_REG);

	/* disable KEYADC gating */
	reg_val = readl(gpadc_bgr_base);
	reg_val &= ~(1 << 0);
	writel(reg_val, gpadc_bgr_base);

	return 0;
}

void sunxi_clock_init_uart(int port)
{
	u32 i, reg;

	/* reset */
	reg = readl(CCMU_UART_BGR_REG);
	reg &= ~(1<<(CCM_UART_RST_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
	for (i = 0; i < 100; i++)
		;
	reg |= (1 << (CCM_UART_RST_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
	/* gate */
	reg = readl(CCMU_UART_BGR_REG);
	reg &= ~(1<<(CCM_UART_GATING_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
	for (i = 0; i < 100; i++)
		;
	reg |= (1 << (CCM_UART_GATING_OFFSET + port));
	writel(reg, CCMU_UART_BGR_REG);
}
