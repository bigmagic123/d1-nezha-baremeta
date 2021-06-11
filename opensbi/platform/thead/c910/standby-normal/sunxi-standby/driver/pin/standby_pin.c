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
#include "type.h"

#define GPIO_REG_BASE 0x0300B000
#define R_GPIO_REG_BASE 0x07022000
#define GPIO_BLANK_NUM 9
#define R_GPIO_BLANK_NUM 1

#define GPIO_CFG_REG(n, i) ((n < GPIO_BLANK_NUM) ? \
		((volatile void *)(long)(GPIO_REG_BASE + (n) * 0x24 + ((i) >> 3) * 0x4 + 0x00)) : \
		((volatile void *)(long)(R_GPIO_REG_BASE + (n - GPIO_BLANK_NUM) * 0x24 + ((i) >> 3) * 0x4 + 0x00)))


#define GPIO_DATA_REG(n) ((n < GPIO_BLANK_NUM) ? \
		((volatile void *)(long)(GPIO_REG_BASE + (n) * 0x24 + 0x10)) : \
		((volatile void *)(long)(R_GPIO_REG_BASE + (n - GPIO_BLANK_NUM) * 0x24 + 0x10)))

#define GPIO_DRV_REG(n, i) ((n < GPIO_BLANK_NUM) ? \
		((volatile void *)(long)(GPIO_REG_BASE + (n) * 0x24 + ((i) >> 4) * 0x4 + 0x14)) : \
		((volatile void *)(long)(R_GPIO_REG_BASE + (n - GPIO_BLANK_NUM) * 0x24 + ((i) >> 4) * 0x4 + 0x14)))

#define GPIO_PULL_REG(n, i) ((n < GPIO_BLANK_NUM) ? \
		((volatile void *)(long)(GPIO_REG_BASE + (n) * 0x24 + ((i) >> 4) * 0x4 + 0x1C)) : \
		((volatile void *)(long)(R_GPIO_REG_BASE + (n - GPIO_BLANK_NUM) * 0x24 + ((i) >> 4) * 0x4 + 0x1C)))

s32 gpio_set_multi_sel(u32 pin_grp, u32 pin_num, u32 multi_sel)
{
	volatile u32 *addr;
	volatile u32 value;

	/* set multi-select */
	addr = GPIO_CFG_REG(pin_grp, pin_num);
	value = *addr;
	value &= ~(0x7 << ((pin_num & 0x7) * 4));
	value |= (multi_sel << ((pin_num & 0x7) * 4));
	*addr = value;

	return 0;
}

s32 gpio_set_pull(u32 pin_grp, u32 pin_num, u32 pull)
{
	volatile u32 *addr;
	volatile u32 value;

	/* set pull status */
	addr = GPIO_PULL_REG(pin_grp, pin_num);
	value = *addr;
	value &= ~(0x3  << ((pin_num & 0xf) * 2));
	value |=  (pull << ((pin_num & 0xf) * 2));

	*addr = value;

	return 0;
}

s32 gpio_set_drive(u32 pin_grp, u32 pin_num, u32 drive)
{
	volatile u32 *addr;
	volatile u32 value;

	/* set drive level */
	addr = GPIO_DRV_REG(pin_grp, pin_num);
	value = *addr;
	value &= ~(0x3 << ((pin_num & 0xf) * 2));
	value |= (drive << ((pin_num & 0xf) * 2));

	*addr = value;

	return 0;
}

s32 gpio_write_data(u32 pin_grp, u32 pin_num, u32 data)
{
	volatile u32 *addr;
	volatile u32 value;

	/* write data */
	addr = GPIO_DATA_REG(pin_grp);
	value = *addr;
	value &= ~(0x1 << pin_num);
	value |= ((data & 0x1) << pin_num);

	*addr = value;

	return 0;
}

u32 pin_read_data(u32 pin_grp, u32 pin_num)
{
	volatile u32 value;

	/* read data */
	value = readl(GPIO_DATA_REG(pin_grp));

	return ((value >> pin_num) & 0x1);
}

