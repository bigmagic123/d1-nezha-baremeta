// SPDX-License-Identifier: BSD-2-Clause
/*
 *  standby/clk-sun8iw19p1.h
 *
 * Copyright (c) 2018 Allwinner.
 * 2019-05-05 Written by frank (frank@allwinnertech.com).
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __CLK_SUN8IW19P1_H__
#define __CLK_SUN8IW19P1_H__
#include <type.h>

#define CCMU_BASE 0x02001000
#define PRCM_BASE 0x07010000
#define RTC_BASE  0x07090000

#define PLL_CPUX_CTRL			(CCMU_BASE + 0x0)
#define PLL_ENABLE_MASK			(0x1 << 31)
#define PLL_ENABLE_BIT			(0x1 << 31)
#define PLL_LOCK_ENABLE_BIT		(0x1 << 29)
#define PLL_LOCK_BIT			(0x1 << 28)
#define CPUX_P_MASK				(0x3 << 16)
#define CPUX_N_MASK				(0xff << 8)
#define CPUX_K_MASK				(0x3 << 4)
#define CPUX_M_MASK				(0x3 << 0)
#define CPUX_FACTOR_MASK	(CPUX_P_MASK | CPUX_N_MASK | CPUX_K_MASK | CPUX_M_MASK)

#define CPUX_AXI_CONFIG			(CCMU_BASE + 0x500)
#define CPUX_CLK_SRC_MASK		(0x3 << 24)
#define OSC24M					(0x0 << 24)
#define RTC_32K					(0x1 << 24)
#define RC16M					(0x2 << 24)
#define PLL_CPUX				(0x3 << 24)
#define PLL_PERIO				(0x4 << 24)
#define AXI_CLK_DIV_RATIO_MASK	((0x3 <<8) | 0x3)
#define CPU_AXI_MASK		(CPUX_CLK_SRC_MASK | AXI_CLK_DIV_RATIO_MASK)

#define AHB1_AHB2_CONFIG		(CCMU_BASE + 0x510)
#define AHB1_CLK_SRC_MASK		(0x3 << 24)
#define AHB1_OSC24M				(0x0 << 24)
#define AHB1_RTC_32K				(0x1 << 24)
#define AHB1_RC16M				(0x2 << 24)
#define AHB1_PLL_PERIO				(0x3 << 24)
#define AHB1_N_MASK			(0x3 << 8)
#define AHB1_M_MASK			(0x3 << 0)
#define AHB1_AHB2_MASK		(AHB1_CLK_SRC_MASK | AHB1_N_MASK | AHB1_M_MASK)

#define AHB3_CONFIG			(CCMU_BASE + 0x51c)
#define AHB3_CLK_SRC_MASK		(0x3 << 24)
#define AHB3_OSC24M				(0x0 << 24)
#define AHB3_RTC_32K				(0x1 << 24)
#define AHB3_PSI				(0x2 << 24)
#define AHB3_PLL_PERIO				(0x3 << 24)
#define AHB3_N_MASK			(0x3 << 8)
#define AHB3_M_MASK			(0x3 << 0)
#define AHB3_MASK		(AHB3_CLK_SRC_MASK | AHB3_N_MASK | AHB3_M_MASK)

#define APB1_CONFIG				(CCMU_BASE + 0x520)
#define APB1_CLK_SRC_MASK		(0x3 << 24)
#define APB1_OSC24M				(0x0 << 24)
#define APB1_RTC_32K				(0x1 << 24)
#define APB1_PSI				(0x2 << 24)
#define APB1_PERI0				(0x3 << 24)
#define APB1_N_MASK			(0x3 << 8)
#define APB1_M_MASK			(0x3 << 0)
#define APB1_MASK			(APB1_CLK_SRC_MASK | APB1_N_MASK | APB1_M_MASK)

#define APB2_CONFIG				(CCMU_BASE + 0x524)
#define APB2_CLK_SRC_MASK		(0x3 << 24)
#define APB2_OSC24M				(0x0 << 24)
#define APB2_RTC_32K				(0x1 << 24)
#define APB2_PSI				(0x2 << 24)
#define APB2_PERI0				(0x3 << 24)
#define APB2_N_MASK			(0x3 << 8)
#define APB2_M_MASK			(0x3 << 0)
#define APB2_MASK			(APB2_CLK_SRC_MASK | APB2_N_MASK | APB2_M_MASK)

#define CPUS_CFG_REG			(PRCM_BASE + 0x000)
#define CPUS_CLK_SRC_MASK		(0x3 << 24)
#define CPUS_OSC24M				(0x0 << 24)
#define CPUS_RTC_32K				(0x1 << 24)
#define CPUS_RC16M				(0x2 << 24)
#define CPUS_PLL_PERIO				(0x3 << 24)
#define CPUS_DIV_N_MASK			(0x3 << 8)
#define CPUS_DIV_M_MASK			(0x1f << 0)
#define CPUS_CLK_DIV_RATIO_MASK		(CPUS_DIV_N_MASK | CPUS_DIV_M_MASK)
#define CPUS_CLK_MASK			(CPUS_CLK_SRC_MASK | CPUS_DIV_N_MASK | CPUS_DIV_M_MASK)

// #define APBS1_CFG_REG			(PRCM_BASE + 0x00c)
#define APBS1_M_MASK			(0x3 << 0)
#define APBS1_DIV_RATIO_MASK		(APBS1_M_MASK)
#define APBS1_MASK			(APBS1_M_MASK)

#define APBS2_CFG_REG			(PRCM_BASE + 0x010)
#define APBS2_CLK_SRC_MASK		(0x3 << 24)
#define APBS2_OSC24M				(0x0 << 24)
#define APBS2_RTC_32K				(0x1 << 24)
#define APBS2_RC16M				(0x2 << 24)
#define APBS2_PERI0				(0x3 << 24)
#define APBS2_N_MASK			(0x3 << 8)
#define APBS2_M_MASK			(0x1f << 0)
#define APBS2_MASK			(APBS2_CLK_SRC_MASK | APBS2_N_MASK | APBS2_M_MASK)


#define XO_CTRL_REG 			(RTC_BASE + 0x160)
#define XO_EN					(0x1 << 1)

// #define PLL_CTRL_REG1 			(RTC_BASE + 0x228)
#define KEY_FIELD_MASK			(0xff << 24)
#define KEY_FIELD_KEY			(0xa7 << 24)
#define HOSC_EN					(0x1 << 2)
#define LDO_EN					(0x1 << 0)

#define TIMESTAMP_STA_BASE		0x08110000
#define CNT_LOW_REG			(TIMESTAMP_STA_BASE + 0x0)
#define CNT_HIGH_REG			(TIMESTAMP_STA_BASE + 0x4)

#define RTC_GENERAL(n) 			(RTC_BASE + 0x100 + ((n)*4))
#define STANDBY_STATUS_REG 		RTC_GENERAL(3)
#define DRAM_CRC_EN_REG			RTC_GENERAL(0)
#define DRAM_CRC_START_REG		RTC_GENERAL(6)
#define DRAM_CRC_LEN_REG		RTC_GENERAL(7)
#define STANDBY_SUPER_FLAG_REG		(RTC_BASE + 0x1f8)
#define STANDBY_SUPER_ADDR_REG		(RTC_BASE + 0x1fc)

#define STANDBY_START			(0x00)
#define SUPER_STANDBY_FLAG		(0x10000)
#define DRAM_CRC_MAGIC			(0x76543210)

#define SUNXI_CCM_TWI_BGR_REG		(CCMU_BASE + 0x91C)
#define SUNXI_PRCM_R_TWI_BGR_REG	(PRCM_BASE + 0x19C)

static inline void save_mem_status(volatile u32 val)
{
        writel(val, (void *)STANDBY_STATUS_REG);
}

static inline void set_dram_crc_paras(u32 crc_enable, u32 crc_start, u32 crc_len)
{
        /* high 28 bits for magic number, low 4 bits for enable */
	writel((crc_enable & 0xf) | DRAM_CRC_MAGIC, (void *)DRAM_CRC_EN_REG);

	writel(crc_start, (void *)DRAM_CRC_START_REG);
        writel(crc_len, (void *)DRAM_CRC_LEN_REG);
}

#endif /* __CLK_SUN8IW19P1_H__ */
