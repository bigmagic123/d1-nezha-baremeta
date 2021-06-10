/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: MMC driver for general mmc operations
 * Author: Aaron <leafy.myeh@allwinnertech.com>
 * Date: 2012-2-3 14:18:18
 */

#ifndef _MMC_DEF_H_
#define _MMC_DEF_H_

#include <common.h>
#include <asm/io.h>
#include <arch/clock.h>

#define SUNXI_MMC0_BASE				(SUNXI_SMHC0_BASE)
#define SUNXI_MMC1_BASE				(SUNXI_SMHC1_BASE)
#define SUNXI_MMC2_BASE				(SUNXI_SMHC2_BASE)

#define MAX_MMC_NUM			3

#define MMC_TRANS_BY_DMA
/*#define MMC_DEBUG*/
#define MMC_REG_FIFO_OS		(0x200U)

#define MMC_REG_BASE		SUNXI_MMC0_BASE
#define CCMU_HCLKGATE0_BASE 	CCMU_SMHC_BGR_REG
#define CCMU_HCLKRST0_BASE 	CCMU_SMHC_BGR_REG
#define CCMU_MMC0_CLK_BASE 	CCMU_SDMMC0_CLK_REG
#define CCMU_MMC2_CLK_BASE 	CCMU_SDMMC2_CLK_REG

/*#define CCMU_PLL5_CLK_BASE    0x01c20020*/
#define __mmc_be32_to_cpu(x)	((0x000000ff&((x)>>24)) | (0x0000ff00&((x)>>8)) | 			\
							 (0x00ff0000&((x)<<8)) | (0xff000000&((x)<<24)))
/*change address to iomem pointer*/
#define IOMEM_ADDR(addr) ((volatile void __iomem *)((phys_addr_t)(addr)))
#define PT_TO_U32(p)   ((u32)((phys_addr_t)(p)))
#define PT_TO_U(p)   ((phys_addr_t)(p))
#define WR_MB() 	wmb()

#define  OSAL_CacheRangeFlush(__s, __l, __a)  flush_dcache_range(PT_TO_U(__s), PT_TO_U(__s)+__l - 1)
#define  OSAL_CacheRangeInvaild(__s, __l, __a)  invalidate_dcache_range(PT_TO_U(__s), PT_TO_U(__s)+__l - 1)

#ifndef NULL
#define NULL (void *)0
#endif

#ifdef MMC_DEBUG
#define mmcinfo(fmt...)	printf("[mmc]: "fmt)
#define mmcdbg(fmt...)	printf("[mmc]: "fmt)
#define mmcmsg(fmt...)	printf(fmt)
#else
#define mmcinfo(fmt...)	printf("[mmc]: "fmt)
#define mmcdbg(fmt...)
#define mmcmsg(fmt...)
#endif

/*#define readb(addr)           (*((volatile unsigned char  *)(addr)))*/
/*#define readw(addr)           (*((volatile unsigned short *)(addr)))*/
/*#define readl(addr)           (*((volatile unsigned long  *)(addr)))*/
/*#define writeb(v, addr)       (*((volatile unsigned char  *)(addr)) = (unsigned char)(v))*/
/*#define writew(v, addr)       (*((volatile unsigned short *)(addr)) = (unsigned short)(v))*/
/*#define writel(v, addr)       (*((volatile unsigned long  *)(addr)) = (unsigned long)(v))*/

#define DMAC_DES_BASE_IN_SRAM		(0x20000 + 0xC000)
#define DMAC_DES_BASE_IN_SDRAM		(0x42000000)
#define DRAM_START_ADDR				(0x40000000)

#define DRIVER_VER  "2021-04-2 16:45"

#endif				/* _MMC_H_ */
