// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/delay.h
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
#ifndef __DELAY_H__
#define __DELAY_H__

void udelay(unsigned int us);
void mdelay(unsigned int ms);
void standby_delay_cycle(unsigned int cycle);

#endif /* __DELAY_H__ */
