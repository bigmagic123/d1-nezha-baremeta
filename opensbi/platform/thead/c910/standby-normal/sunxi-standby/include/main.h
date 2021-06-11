// SPDX-License-Identifier: BSD-2-Clause
/*
 *  drivers/standby/main.h
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
#ifndef __MAIN_H__
#define __MAIN_H__

u64 save_sp(void);
void restore_sp(u64 sp);
extern char *__bss_start;
extern char *__bss_end;

#endif /*__MAIN_H__*/
