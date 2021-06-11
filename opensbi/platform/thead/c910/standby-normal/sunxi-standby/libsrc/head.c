// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/head.c
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

#include "head.h"

const sram_head_t  sram_head =
{
	.dram_para = {
		672,         /* dram_clk    */
		3,           /* dram_type   */
		0x003F3FDD,  /* dram_zq     */
		1,           /* dram_odt_en */
		0x10f41000,  /* dram_para1  */
		0x00001200,  /* dram_para2  */
		0x1A50,      /* dram_mr0    */
		0x40,        /* dram_mr1    */
		0x10,        /* dram_mr2    */
		0,           /* dram_mr3    */
		0x04E214EA,  /* dram_tpr0   */
		0x004214AD,  /* dram_tpr1   */
		0x10A75030,  /* dram_tpr2   */
		0,           /* dram_tpr3   */
		0,           /* dram_tpr4   */
		0,           /* dram_tpr5   */
		0,           /* dram_tpr6   */
		0,           /* dram_tpr7   */
		0,           /* dram_tpr8   */
		0,           /* dram_tpr9   */
		0,           /* dram_tpr10  */
		0,           /* dram_tpr11  */
		168,         /* dram_tpr12  */
		0x823       /* dram_tpr13  */
	},
	.entry = 0,
};
