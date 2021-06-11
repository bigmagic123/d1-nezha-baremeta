// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/clk.h
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
#ifndef __CLK_H__
#define __CLK_H__

#include "clk-plat.h"
#include "head.h"

void system_suspend(sram_head_t *para);
void system_restore(sram_head_t *para);

#endif /* __CLK_H__ */
