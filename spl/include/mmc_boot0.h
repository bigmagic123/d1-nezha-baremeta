/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
#ifndef _SUNXI_MMC_BOOT0_H
#define _SUNXI_MMC_BOOT0_H

#define CARD_TYPE_MMC	0x8000000
#define CARD_TYPE_SD	0x8000001

void set_mmc_para(int smc_no, void *sdly_addr, phys_addr_t uboot_base);
int get_card_type(void);
unsigned long mmc_bread(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
int sunxi_mmc_init(int sdc_no, unsigned bus_width, const normal_gpio_cfg *gpio_info, int offset);
int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);

#endif /* _SUNXI_MMC_BOOT0_H */
