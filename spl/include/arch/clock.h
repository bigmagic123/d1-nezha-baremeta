/*
 * (C) Copyright 2007-2011
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#ifndef _SUNXI_CLOCK_H
#define _SUNXI_CLOCK_H

#include <linux/types.h>

#define CLK_GATE_OPEN			0x1
#define CLK_GATE_CLOSE			0x0

/* clock control module regs definition */
#if defined(CONFIG_ARCH_SUN50IW3)
#include <arch/clock_sun50iw3.h>
#elif defined(CONFIG_ARCH_SUN50IW9)
#include <arch/clock_sun50iw9.h>
#elif defined(CONFIG_ARCH_SUN8IW18)
#include <arch/clock_sun8iw18.h>
#elif defined(CONFIG_ARCH_SUN8IW16)
#include <arch/clock_sun8iw16.h>
#elif defined(CONFIG_ARCH_SUN8IW19)
#include <arch/clock_sun8iw19.h>
#elif defined(CONFIG_ARCH_SUN50IW10)
#include <arch/clock_sun50iw10.h>
#elif defined(CONFIG_ARCH_SUN8IW15)
#include <arch/clock_sun8iw15.h>
#elif defined(CONFIG_ARCH_SUN8IW7)
#include <arch/clock_sun8iw7.h>
#elif defined(CONFIG_ARCH_SUN50IW11)
#include <arch/clock_sun50iw11.h>
#elif defined(CONFIG_ARCH_SUN50IW12)
#include <arch/clock_sun50iw12.h>
#elif defined(CONFIG_ARCH_SUN20IW1)
#include <arch/clock_sun20iw1.h>
#elif defined(CONFIG_ARCH_SUN8IW20)
#include <arch/clock_sun8iw20.h>
#elif defined(CONFIG_ARCH_SUN8IW21)
#include <arch/clock_sun8iw21.h>
#elif defined(CONFIG_ARCH_SUN50IW5)
#include <arch/clock_sun50iw5.h>
#else
#error "Unsupported plat"
#endif

#ifdef CONFIG_SUNXI_VERSION1
struct sunxi_ccm_reg {
	u32 pll1_c0_cfg; /* 0x00 c1cpu# pll control */
	u32 pll1_c1_cfg; /* 0x04 c1cpu# pll control */
	u32 pll2_cfg; /* 0x08 pll2 audio control */
	u32 reserved1;
	u32 pll3_cfg; /* 0x10 pll3 video0 control */
	u32 reserved2;
	u32 pll4_cfg; /* 0x18 pll4 ve control */
	u32 reserved3;
	u32 pll5_cfg; /* 0x20 pll5 ddr control */
	u32 reserved4;
	u32 pll6_cfg; /* 0x28 pll6 peripheral control */
	u32 reserved5[3]; /* 0x2c */
	u32 pll7_cfg; /* 0x38 pll7 gpu control */
	u32 reserved6[2]; /* 0x3c */
	u32 pll8_cfg; /* 0x44 pll8 hsic control */
	u32 pll9_cfg; /* 0x48 pll9 de control */
	u32 pll10_cfg; /* 0x4c pll10 video1 control */
	u32 cpu_axi_cfg; /* 0x50 CPU/AXI divide ratio */
	u32 ahb1_apb1_div; /* 0x54 AHB1/APB1 divide ratio */
	u32 apb2_div; /* 0x58 APB2 divide ratio */
	u32 ahb2_div; /* 0x5c AHB2 divide ratio */
	u32 ahb_gate0; /* 0x60 ahb module clock gating 0 */
	u32 ahb_gate1; /* 0x64 ahb module clock gating 1 */
	u32 apb1_gate; /* 0x68 apb1 module clock gating 3 */
	u32 apb2_gate; /* 0x6c apb2 module clock gating 4 */
	u32 reserved7[2]; /* 0x70 */
	u32 cci400_cfg; /* 0x78 cci400 clock configuration A83T only */
	u32 reserved8; /* 0x7c */
	u32 nand0_clk_cfg; /* 0x80 nand clock control */
	u32 reserved9; /* 0x84 */
	u32 sd0_clk_cfg; /* 0x88 sd0 clock control */
	u32 sd1_clk_cfg; /* 0x8c sd1 clock control */
	u32 sd2_clk_cfg; /* 0x90 sd2 clock control */
	u32 sd3_clk_cfg; /* 0x94 sd3 clock control */
	u32 reserved10; /* 0x98 */
	u32 ss_clk_cfg; /* 0x9c security system clock control */
	u32 spi0_clk_cfg; /* 0xa0 spi0 clock control */
	u32 spi1_clk_cfg; /* 0xa4 spi1 clock control */
	u32 reserved11[2]; /* 0xa8 */
	u32 i2s0_clk_cfg; /* 0xb0 I2S0 clock control */
	u32 i2s1_clk_cfg; /* 0xb4 I2S1 clock control */
	u32 i2s2_clk_cfg; /* 0xb8 I2S2 clock control */
	u32 tdm_clk_cfg; /* 0xbc TDM clock control */
	u32 spdif_clk_cfg; /* 0xc0 SPDIF clock control */
	u32 reserved12[2]; /* 0xc4 */
	u32 usb_clk_cfg; /* 0xcc USB clock control */
	u32 reserved13[9]; /* 0xd0 */
	u32 dram_clk_cfg; /* 0xf4 DRAM configuration clock control */
	u32 dram_pll_cfg; /* 0xf8 PLL_DDR cfg register */
	u32 mbus_reset; /* 0xfc MBUS reset control */
	u32 dram_clk_gate; /* 0x100 DRAM module gating */
	u32 reserved14[5]; /* 0x104 BE0 */
	u32 lcd0_clk_cfg; /* 0x118 LCD0 module clock */
	u32 lcd1_clk_cfg; /* 0x11c LCD1 module clock */
	u32 reserved15[4]; /* 0x120 */
	u32 mipi_csi_clk_cfg; /* 0x130 MIPI CSI module clock */
	u32 csi_clk_cfg; /* 0x134 CSI module clock */
	u32 reserved16; /* 0x138 */
	u32 ve_clk_cfg; /* 0x13c VE module clock */
	u32 reserved17; /* 0x140 */
	u32 avs_clk_cfg; /* 0x144 AVS module clock */
	u32 reserved18[2]; /* 0x148 */
	u32 hdmi_clk_cfg; /* 0x150 HDMI module clock */
	u32 hdmi_slow_clk_cfg; /* 0x154 HDMI slow module clock */
	u32 reserved19; /* 0x158 */
	u32 mbus_clk_cfg; /* 0x15c MBUS module clock */
	u32 reserved20[2]; /* 0x160 */
	u32 mipi_dsi_clk_cfg; /* 0x168 MIPI DSI clock control */
	u32 reserved21[13]; /* 0x16c */
	u32 gpu_core_clk_cfg; /* 0x1a0 GPU core clock config */
	u32 gpu_mem_clk_cfg; /* 0x1a4 GPU memory clock config */
	u32 gpu_hyd_clk_cfg; /* 0x1a8 GPU HYD clock config */
	u32 reserved22[21]; /* 0x1ac */
	u32 pll_stable0; /* 0x200 PLL stable time 0 */
	u32 pll_stable1; /* 0x204 PLL stable time 1 */
	u32 reserved23; /* 0x208 */
	u32 pll_stable_status; /* 0x20c PLL stable status register */
	u32 reserved24[4]; /* 0x210 */
	u32 pll1_c0_bias_cfg; /* 0x220 PLL1 c0cpu# Bias config */
	u32 pll2_bias_cfg; /* 0x224 PLL2 audio Bias config */
	u32 pll3_bias_cfg; /* 0x228 PLL3 video Bias config */
	u32 pll4_bias_cfg; /* 0x22c PLL4 ve Bias config */
	u32 pll5_bias_cfg; /* 0x230 PLL5 ddr Bias config */
	u32 pll6_bias_cfg; /* 0x234 PLL6 periph Bias config */
	u32 pll1_c1_bias_cfg; /* 0x238 PLL1 c1cpu# Bias config */
	u32 pll8_bias_cfg; /* 0x23c PLL7 Bias config */
	u32 reserved25; /* 0x240 */
	u32 pll9_bias_cfg; /* 0x244 PLL9 hsic Bias config */
	u32 de_bias_cfg; /* 0x248 display engine Bias config */
	u32 video1_bias_cfg; /* 0x24c pll video1 bias register */
	u32 c0_tuning_cfg; /* 0x250 pll c0cpu# tuning register */
	u32 c1_tuning_cfg; /* 0x254 pll c1cpu# tuning register */
	u32 reserved26[11]; /* 0x258 */
	u32 pll2_pattern_cfg0; /* 0x284 PLL2 Pattern register 0 */
	u32 pll3_pattern_cfg0; /* 0x288 PLL3 Pattern register 0 */
	u32 reserved27; /* 0x28c */
	u32 pll5_pattern_cfg0; /* 0x290 PLL5 Pattern register 0*/
	u32 reserved28[4]; /* 0x294 */
	u32 pll2_pattern_cfg1; /* 0x2a4 PLL2 Pattern register 1 */
	u32 pll3_pattern_cfg1; /* 0x2a8 PLL3 Pattern register 1 */
	u32 reserved29; /* 0x2ac */
	u32 pll5_pattern_cfg1; /* 0x2b0 PLL5 Pattern register 1 */
	u32 reserved30[3]; /* 0x2b4 */
	u32 ahb_reset0_cfg; /* 0x2c0 AHB1 Reset 0 config */
	u32 ahb_reset1_cfg; /* 0x2c4 AHB1 Reset 1 config */
	u32 ahb_reset2_cfg; /* 0x2c8 AHB1 Reset 2 config */
	u32 reserved31;
	u32 ahb_reset3_cfg; /* 0x2d0 AHB1 Reset 3 config */
	u32 reserved32; /* 0x2d4 */
	u32 apb2_reset_cfg; /* 0x2d8 BUS Reset 4 config */
};

#else
struct sunxi_ccm_reg {
	u32 pll1_cfg;		/* 0x000 pll1 (cpux) control */
	u8 reserved_0x004[12];
	u32 pll5_cfg;		/* 0x010 pll5 (ddr) control */
	u8 reserved_0x014[12];
	u32 pll6_cfg;		/* 0x020 pll6 (periph0) control */
	u8 reserved_0x020[4];
	u32 pll_periph1_cfg;	/* 0x028 pll periph1 control */
	u8 reserved_0x028[4];
	u32 pll7_cfg;		/* 0x030 pll7 (gpu) control */
	u8 reserved_0x034[12];
	u32 pll3_cfg;		/* 0x040 pll3 (video0) control */
	u8 reserved_0x044[4];
	u32 pll_video1_cfg;	/* 0x048 pll video1 control */
	u8 reserved_0x04c[12];
	u32 pll4_cfg;		/* 0x058 pll4 (ve) control */
	u8 reserved_0x05c[4];
	u32 pll10_cfg;		/* 0x060 pll10 (de) control */
	u8 reserved_0x064[12];
	u32 pll9_cfg;		/* 0x070 pll9 (hsic) control */
	u8 reserved_0x074[4];
	u32 pll2_cfg;		/* 0x078 pll2 (audio) control */
	u8 reserved_0x07c[148];
	u32 pll5_pat;		/* 0x110 pll5 (ddr) pattern */
	u8 reserved_0x114[20];
	u32 pll_periph1_pat0;	/* 0x128 pll periph1 pattern0 */
	u32 pll_periph1_pat1;	/* 0x12c pll periph1 pattern1 */
	u32 pll7_pat0;		/* 0x130 pll7 (gpu) pattern0 */
	u32 pll7_pat1;		/* 0x134 pll7 (gpu) pattern1 */
	u8 reserved_0x138[8];
	u32 pll3_pat0;		/* 0x140 pll3 (video0) pattern0 */
	u32 pll3_pat1;		/* 0x144 pll3 (video0) pattern1 */
	u32 pll_video1_pat0;	/* 0x148 pll video1 pattern0 */
	u32 pll_video1_pat1;	/* 0x14c pll video1 pattern1 */
	u8 reserved_0x150[8];
	u32 pll4_pat0;		/* 0x158 pll4 (ve) pattern0 */
	u32 pll4_pat1;		/* 0x15c pll4 (ve) pattern1 */
	u32 pll10_pat0;		/* 0x160 pll10 (de) pattern0 */
	u32 pll10_pat1;		/* 0x164 pll10 (de) pattern1 */
	u8 reserved_0x168[8];
	u32 pll9_pat0;		/* 0x170 pll9 (hsic) pattern0 */
	u32 pll9_pat1;		/* 0x174 pll9 (hsic) pattern1 */
	u32 pll2_pat0;		/* 0x178 pll2 (audio) pattern0 */
	u32 pll2_pat1;		/* 0x17c pll2 (audio) pattern1 */
	u8 reserved_0x180[384];
	u32 pll1_bias;		/* 0x300 pll1 (cpux) bias */
	u8 reserved_0x304[12];
	u32 pll5_bias;		/* 0x310 pll5 (ddr) bias */
	u8 reserved_0x314[12];
	u32 pll6_bias;		/* 0x320 pll6 (periph0) bias */
	u8 reserved_0x324[4];
	u32 pll_periph1_bias;	/* 0x328 pll periph1 bias */
	u8 reserved_0x32c[4];
	u32 pll7_bias;		/* 0x330 pll7 (gpu) bias */
	u8 reserved_0x334[12];
	u32 pll3_bias;		/* 0x340 pll3 (video0) bias */
	u8 reserved_0x344[4];
	u32 pll_video1_bias;	/* 0x348 pll video1 bias */
	u8 reserved_0x34c[12];
	u32 pll4_bias;		/* 0x358 pll4 (ve) bias */
	u8 reserved_0x35c[4];
	u32 pll10_bias;		/* 0x360 pll10 (de) bias */
	u8 reserved_0x364[12];
	u32 pll9_bias;		/* 0x370 pll9 (hsic) bias */
	u8 reserved_0x374[4];
	u32 pll2_bias;		/* 0x378 pll2 (audio) bias */
	u8 reserved_0x37c[132];
	u32 pll1_tun;		/* 0x400 pll1 (cpux) tunning */
	u8 reserved_0x404[252];
	u32 cpu_axi_cfg;	/* 0x500 CPUX/AXI clock control*/
	u8 reserved_0x504[12];
	u32 psi_ahb1_ahb2_cfg;	/* 0x510 PSI/AHB1/AHB2 clock control */
	u8 reserved_0x514[8];
	u32 ahb3_cfg;		/* 0x51c AHB3 clock control */
	u32 apb1_cfg;		/* 0x520 APB1 clock control */
	u32 apb2_cfg;		/* 0x524 APB2 clock control */
	u8 reserved_0x528[24];
	u32 mbus_cfg;		/* 0x540 MBUS clock control */
	u8 reserved_0x544[188];
	u32 de_clk_cfg;		/* 0x600 DE clock control */
	u8 reserved_0x604[8];
	u32 de_gate_reset;	/* 0x60c DE gate/reset control */
	u8 reserved_0x610[16];
	u32 di_clk_cfg;		/* 0x620 DI clock control */
	u8 reserved_0x024[8];
	u32 di_gate_reset;	/* 0x62c DI gate/reset control */
	u8 reserved_0x630[64];
	u32 gpu_clk_cfg;	/* 0x670 GPU clock control */
	u8 reserved_0x674[8];
	u32 gpu_gate_reset;	/* 0x67c GPU gate/reset control */
	u32 ce_clk_cfg;		/* 0x680 CE clock control */
	u8 reserved_0x684[8];
	u32 ce_gate_reset;	/* 0x68c CE gate/reset control */
	u32 ve_clk_cfg;		/* 0x690 VE clock control */
	u8 reserved_0x694[8];
	u32 ve_gate_reset;	/* 0x69c VE gate/reset control */
	u8 reserved_0x6a0[16];
	u32 emce_clk_cfg;	/* 0x6b0 EMCE clock control */
	u8 reserved_0x6b4[8];
	u32 emce_gate_reset;	/* 0x6bc EMCE gate/reset control */
	u32 vp9_clk_cfg;	/* 0x6c0 VP9 clock control */
	u8 reserved_0x6c4[8];
	u32 vp9_gate_reset;	/* 0x6cc VP9 gate/reset control */
	u8 reserved_0x6d0[60];
	u32 dma_gate_reset;	/* 0x70c DMA gate/reset control */
	u8 reserved_0x710[12];
	u32 msgbox_gate_reset;	/* 0x71c Message Box gate/reset control */
	u8 reserved_0x720[12];
	u32 spinlock_gate_reset;/* 0x72c Spinlock gate/reset control */
	u8 reserved_0x730[12];
	u32 hstimer_gate_reset;	/* 0x73c HS Timer gate/reset control */
	u32 avs_gate_reset;	/* 0x740 AVS gate/reset control */
	u8 reserved_0x744[72];
	u32 dbgsys_gate_reset;	/* 0x78c Debugging system gate/reset control */
	u8 reserved_0x790[12];
	u32 psi_gate_reset;	/* 0x79c PSI gate/reset control */
	u8 reserved_0x7a0[12];
	u32 pwm_gate_reset;	/* 0x7ac PWM gate/reset control */
	u8 reserved_0x7b0[12];
	u32 iommu_gate_reset;	/* 0x7bc IOMMU gate/reset control */
	u8 reserved_0x7c0[64];
	u32 dram_clk_cfg;		/* 0x800 DRAM clock control */
	u32 mbus_gate;		/* 0x804 MBUS gate control */
	u8 reserved_0x808[4];
	u32 dram_gate_reset;	/* 0x80c DRAM gate/reset control */
	u32 nand0_clk_cfg;	/* 0x810 NAND0 clock control */
	u32 nand1_clk_cfg;	/* 0x814 NAND1 clock control */
	u8 reserved_0x818[20];
	u32 nand_gate_reset;	/* 0x82c NAND gate/reset control */
	u32 sd0_clk_cfg;	/* 0x830 MMC0 clock control */
	u32 sd1_clk_cfg;	/* 0x834 MMC1 clock control */
	u32 sd2_clk_cfg;	/* 0x838 MMC2 clock control */
	u8 reserved_0x83c[16];
	u32 sd_gate_reset;	/* 0x84c MMC gate/reset control */
	u8 reserved_0x850[188];
	u32 uart_gate_reset;	/* 0x90c UART gate/reset control */
	u8 reserved_0x910[12];
	u32 twi_gate_reset;	/* 0x91c I2C gate/reset control */
	u8 reserved_0x920[28];
	u32 scr_gate_reset;	/* 0x93c SCR gate/reset control */
	u32 spi0_clk_cfg;	/* 0x940 SPI0 clock control */
	u32 spi1_clk_cfg;	/* 0x944 SPI1 clock control */
	u8 reserved_0x948[36];
	u32 spi_gate_reset;	/* 0x96c SPI gate/reset control */
	u8 reserved_0x970[12];
	u32 emac_gate_reset;	/* 0x97c EMAC gate/reset control */
	u8 reserved_0x980[48];
	u32 ts_clk_cfg;		/* 0x9b0 TS clock control */
	u8 reserved_0x9b4[8];
	u32 ts_gate_reset;	/* 0x9bc TS gate/reset control */
	u32 irtx_clk_cfg;	/* 0x9c0 IR TX clock control */
	u8 reserved_0x9c4[8];
	u32 irtx_gate_reset;	/* 0x9cc IR TX gate/reset control */
	u8 reserved_0x9d0[44];
	u32 ths_gate_reset;	/* 0x9fc THS gate/reset control */
	u8 reserved_0xa00[12];
	u32 i2s3_clk_cfg;	/* 0xa0c I2S3 clock control */
	u32 i2s0_clk_cfg;	/* 0xa10 I2S0 clock control */
	u32 i2s1_clk_cfg;	/* 0xa14 I2S1 clock control */
	u32 i2s2_clk_cfg;	/* 0xa18 I2S2 clock control */
	u32 i2s_gate_reset;	/* 0xa1c I2S gate/reset control */
	u32 spdif_clk_cfg;	/* 0xa20 SPDIF clock control */
	u8 reserved_0xa24[8];
	u32 spdif_gate_reset;	/* 0xa2c SPDIF gate/reset control */
	u8 reserved_0xa30[16];
	u32 dmic_clk_cfg;	/* 0xa40 DMIC clock control */
	u8 reserved_0xa44[8];
	u32 dmic_gate_reset;	/* 0xa4c DMIC gate/reset control */
	u8 reserved_0xa50[16];
	u32 ahub_clk_cfg;	/* 0xa60 Audio HUB clock control */
	u8 reserved_0xa64[8];
	u32 ahub_gate_reset;	/* 0xa6c Audio HUB gate/reset control */
	u32 usb0_clk_cfg;	/* 0xa70 USB0(OTG) clock control */
	u32 usb1_clk_cfg;	/* 0xa74 USB1(XHCI) clock control */
	u8 reserved_0xa78[4];
	u32 usb3_clk_cfg;	/* 0xa78 USB3 clock control */
	u8 reserved_0xa80[12];
	u32 usb_gate_reset;	/* 0xa8c USB gate/reset control */
	u8 reserved_0xa90[32];
	u32 pcie_ref_clk_cfg;	/* 0xab0 PCIE REF clock control */
	u32 pcie_axi_clk_cfg;	/* 0xab4 PCIE AXI clock control */
	u32 pcie_aux_clk_cfg;	/* 0xab8 PCIE AUX clock control */
	u32 pcie_gate_reset;	/* 0xabc PCIE gate/reset control */
	u8 reserved_0xac0[64];
	u32 hdmi_clk_cfg;	/* 0xb00 HDMI clock control */
	u32 hdmi_slow_clk_cfg;	/* 0xb04 HDMI slow clock control */
	u8 reserved_0xb08[8];
	u32 hdmi_cec_clk_cfg;	/* 0xb10 HDMI CEC clock control */
	u8 reserved_0xb14[8];
	u32 hdmi_gate_reset;	/* 0xb1c HDMI gate/reset control */
	u8 reserved_0xb20[60];
	u32 tcon_top_gate_reset;/* 0xb5c TCON TOP gate/reset control */
	u32 tcon_lcd0_clk_cfg;	/* 0xb60 TCON LCD0 clock control */
	u8 reserved_0xb64[24];
	u32 tcon_lcd_gate_reset;/* 0xb7c TCON LCD gate/reset control */
	u32 tcon_tv0_clk_cfg;	/* 0xb80 TCON TV0 clock control */
	u8 reserved_0xb84[24];
	u32 tcon_tv_gate_reset;	/* 0xb9c TCON TV gate/reset control */
	u8 reserved_0xba0[96];
	u32 csi_misc_clk_cfg;	/* 0xc00 CSI MISC clock control */
	u32 csi_top_clk_cfg;	/* 0xc04 CSI TOP clock control */
	u32 csi_mclk_cfg;	/* 0xc08 CSI Master clock control */
	u8 reserved_0xc0c[32];
	u32 csi_gate_reset;	/* 0xc2c CSI gate/reset control */
	u8 reserved_0xc30[16];
	u32 hdcp_clk_cfg;	/* 0xc40 HDCP clock control */
	u8 reserved_0xc44[8];
	u32 hdcp_gate_reset;	/* 0xc4c HDCP gate/reset control */
	u8 reserved_0xc50[688];
	u32 ccu_sec_switch;	/* 0xf00 CCU security switch */
	u32 pll_lock_dbg_ctrl;	/* 0xf04 PLL lock debugging control */
};

#endif

/* Module gate/reset shift*/
#define RESET_SHIFT                     (16)
#define GATING_SHIFT                    (0)


#ifndef __ASSEMBLY__
void sunxi_board_pll_init(void);
void sunxi_board_clock_reset(void);
void sunxi_clock_init_uart(int port);
#endif
/*key clock*/
int sunxi_clock_init_key(void);
int sunxi_clock_exit_key(void);

#endif /* _SUNXI_CLOCK_H */
