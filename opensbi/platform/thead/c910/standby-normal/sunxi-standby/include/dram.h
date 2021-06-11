// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/dram.h
 *
 * Copyright (c) 2018 Allwinner.
 * 2018-09-14 Written by fanqinghua (fanqinghua@allwinnertech.com).
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __STANDBY_DRAM_
#define __STANDBY_DRAM_

#include "head.h"

extern unsigned int dram_power_save_process(void);
extern unsigned int dram_power_up_process(__dram_para_t *para);
extern void dram_enable_all_master(void);
extern void dram_disable_all_master(void);
extern u32 dram_crc(sram_head_t * para);
extern int dram_enter_selfresh(__dram_para_t * para);
extern int dram_exit_selfresh(__dram_para_t * para);
#endif	/* __STANDBY_DRAM_ */
