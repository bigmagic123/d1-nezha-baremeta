// SPDX-License-Identifier: BSD-2-Clause
/*
 * standby driver for allwinnertech
 *
 * Copyright (C) 2015 allwinnertech Ltd.
 * Author: Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "standby_clk.h"
#include "clk.h"

s32 ccu_set_mclk_onoff(u32 mclk, s32 onoff)
{
	volatile u32 *reg_addr;
	volatile u32 reg_value;

	switch (mclk) {
	case CCU_MOD_CLK_TWI0:
	case CCU_MOD_CLK_TWI1:
	case CCU_MOD_CLK_TWI2:
	case CCU_MOD_CLK_TWI3:
		{
			reg_addr = (volatile u32 *)SUNXI_CCM_TWI_BGR_REG;
			reg_value = *reg_addr;
			reg_value &= ~(0x1 << (mclk - CCU_MOD_CLK_TWI0));
			reg_value |= (onoff << (mclk - CCU_MOD_CLK_TWI0));
			*reg_addr = reg_value;
			return 0;
		}
	case CCU_MOD_CLK_R_TWI0:
	case CCU_MOD_CLK_R_TWI1:
	case CCU_MOD_CLK_R_TWI2:
	case CCU_MOD_CLK_R_TWI3:
		{
			reg_addr = (volatile u32 *)SUNXI_PRCM_R_TWI_BGR_REG;
			reg_value = *reg_addr;
			reg_value &= ~(0x1 << (mclk - CCU_MOD_CLK_R_TWI0));
			reg_value |= (onoff << (mclk - CCU_MOD_CLK_R_TWI0));
			*reg_addr = reg_value;
			return 0;
		}
	default:
		{
			return -1;
		}
	}
}

s32 ccu_set_mclk_reset(u32 mclk, s32 reset)
{
	volatile u32 *reg_addr;
	volatile u32 reg_value;

	switch (mclk) {
	case CCU_MOD_CLK_TWI0:
	case CCU_MOD_CLK_TWI1:
	case CCU_MOD_CLK_TWI2:
	case CCU_MOD_CLK_TWI3:
		{
			reg_addr = (volatile u32 *)SUNXI_CCM_TWI_BGR_REG;
			reg_value = *reg_addr;
			reg_value &= ~(0x1 << (mclk - CCU_MOD_CLK_TWI0 + 16));
			reg_value |= (reset << (mclk - CCU_MOD_CLK_TWI0 + 16));
			*reg_addr = reg_value;
			return 0;
		}
	case CCU_MOD_CLK_R_TWI0:
	case CCU_MOD_CLK_R_TWI1:
	case CCU_MOD_CLK_R_TWI2:
	case CCU_MOD_CLK_R_TWI3:
		{
			reg_addr = (volatile u32 *)SUNXI_PRCM_R_TWI_BGR_REG;
			reg_value = *reg_addr;
			reg_value &= ~(0x1 << (mclk - CCU_MOD_CLK_R_TWI0 + 16));
			reg_value |= (reset << (mclk - CCU_MOD_CLK_R_TWI0 + 16));
			*reg_addr = reg_value;
			return 0;
		}
	default:
		{
			return -1;
		}
	}
}

s32 ccu_reset_module(u32 mclk)
{
	/* module reset method: set as reset valid->set as reset invalid */
	ccu_set_mclk_reset(mclk, CCU_CLK_RESET);
	ccu_set_mclk_reset(mclk, CCU_CLK_NRESET);

	return 0;
}
