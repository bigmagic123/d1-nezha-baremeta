// SPDX-License-Identifier: BSD-2-Clause
#ifndef _STANDBY_CLK_H
#define _STANDBY_CLK_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include "type.h"

/* the clock status of on-off */
typedef enum ccu_clk_onoff {
	CCU_CLK_OFF = 0x0,
	CCU_CLK_ON = 0x1,
} ccu_clk_onff_e;

/* the clock status of reset */
typedef enum ccu_clk_reset {
	CCU_CLK_RESET = 0x0,
	CCU_CLK_NRESET = 0x1,
} ccu_clk_reset_e;

/* module clocks ID */
typedef enum ccu_mod_clk {
	CCU_MOD_CLK_TWI0,
	CCU_MOD_CLK_TWI1,
	CCU_MOD_CLK_TWI2,
	CCU_MOD_CLK_TWI3,
	CCU_MOD_CLK_R_TWI0,
	CCU_MOD_CLK_R_TWI1,
	CCU_MOD_CLK_R_TWI2,
	CCU_MOD_CLK_R_TWI3,
} ccu_mod_clk_e;

extern s32 ccu_set_mclk_onoff(u32 mclk, s32 onoff);
extern s32 ccu_set_mclk_reset(u32 mclk, s32 reset);
extern s32 ccu_reset_module(u32 mclk);

#endif /*_STANDBY_CLK_H*/
