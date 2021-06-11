// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/dram.c
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
#include "type.h"
#include "dram.h"
#include "head.h"

int dram_enter_selfresh(__dram_para_t * para)
{
	int ret = -1;

	dram_disable_all_master();
	ret = dram_power_save_process();

	return ret;
}

int dram_exit_selfresh(__dram_para_t * para)
{
	int ret = -1;

	ret = dram_power_up_process(para);
	dram_enable_all_master();

	return 0;
}

u32 dram_crc(sram_head_t * para)
{
	u32 *pdata = (u32 *)0;
	u32 crc = 0;
	u32 start = 0;
	pdata = (u32 *)(unsigned long)para->crc_start;
	start = (unsigned long)pdata;
	while (pdata < (u32 *)(long)(start + para->crc_len)) {
		crc += *pdata;
		pdata++;
	}

	return crc;
}

