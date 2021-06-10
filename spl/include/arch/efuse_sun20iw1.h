/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */

#ifndef __EFUSE_H__
#define __EFUSE_H__

#include <arch/cpu.h>

#define SID_PRCTL               (SUNXI_SID_BASE + 0x40)
#define SID_PRKEY               (SUNXI_SID_BASE + 0x50)
#define SID_RDKEY               (SUNXI_SID_BASE + 0x60)
#define SJTAG_AT1				(SUNXI_SID_BASE + 0x84)
#define SJTAG_S					(SUNXI_SID_BASE + 0x88)

#define SID_EFUSE               (SUNXI_SID_BASE + 0x200)
#define SID_SECURE_MODE         (SUNXI_SID_BASE + 0xA0)
#define SID_OP_LOCK  (0xAC)

#define SID_WORK_STATUS			(3)

/*efuse power ctl*/
#define EFUSE_HV_SWITCH			(SUNXI_RTC_BASE + 0x204)

#define EFUSE_THS				(0x14)
/* jtag security */
#define EFUSE_LCJS              (0x48)
#define EFUSE_ROTPK             (0x70) /* 0x70-0x8F, 256bits */
#define EFUSE_NV1               (0xD0) /* 0xD0-0xD3, 32 bits */
#define EFUSE_HUK				(0x50)

/*write protect*/
#define EFUSE_WRITE_PROTECT		(0x40)

/*write protect offset*/
#define EFUSE_HUK_PROTECT_OFFSET	(9)

/* size (bit)*/
#define SID_NV1_SIZE			(32)
#define SID_ROTPK_SIZE			(256)
#define SID_HUK_SIZE			(192)


#endif    /*  #ifndef __EFUSE_H__  */
