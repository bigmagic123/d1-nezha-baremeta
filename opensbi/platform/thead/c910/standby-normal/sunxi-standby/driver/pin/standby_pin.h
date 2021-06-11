// SPDX-License-Identifier: BSD-2-Clause
#ifndef _STANDBY_PIN_H
#define _STANDBY_PIN_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include "type.h"

extern s32 gpio_set_multi_sel(u32 pin_grp, u32 pin_num, u32 multi_sel);
extern s32 gpio_set_pull(u32 pin_grp, u32 pin_num, u32 pull);
extern s32 gpio_set_drive(u32 pin_grp, u32 pin_num, u32 drive);
extern s32 gpio_write_data(u32 pin_grp, u32 pin_num, u32 data);
extern u32 gpio_read_data(u32 pin_grp, u32 pin_num);

#endif /*_STANDBY_PIN_H*/
