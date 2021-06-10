/*
 * (C) Copyright 2007-2013
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>

void __iomem *sunxi_get_iobase(unsigned int base)
{
	void __iomem *addr = (void __iomem *)((phys_addr_t)base);
	return addr;
}

unsigned int sunxi_get_lw32_addr(void *base)
{
	unsigned int addr  = (unsigned int)((phys_addr_t)base & 0xffffffff);
	return addr;
}
