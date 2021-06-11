// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/mdelay.c
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
#include "clk.h"
#include "delay.h"
#include "head.h"

#pragma GCC optimize ("O0")
void udelay(u32 us)
{
	while (us--);

	return;
}

void standby_delay_cycle(unsigned int cycle)
{
	udelay(cycle);
}

void mdelay(u32 ms)
{
	while (ms > 10) {
		udelay(10 * 1000);
		ms -= 10;
	}

	udelay(ms * 1000);
	return;
}
