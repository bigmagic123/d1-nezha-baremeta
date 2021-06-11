// SPDX-License-Identifier: BSD-2-Clause
/*
 * For: AXP221		AW1652/1655
 *	AXP221s		AW1652/1655
 *	AXP223		AW1652/1655
 *	AXP227		AW1652/1655
 *
 * Copyright (C) 2015 allwinnertech Ltd.
 * Author: Ming Li <liming@allwinnertech.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AXP22_POWER_H__
#define __AXP22_POWER_H__

/* AXP22 Regulator Enable Registers */
#define AXP22_LDO_DC_EN1	(0X10)
#define AXP22_LDO_DC_EN2	(0X12)
#define AXP22_LDO_DC_EN3	(0X13)
#define AXP22_GPIO0_CTL		(0x90)
#define AXP22_GPIO1_CTL		(0x92)

/* AXP22 Regulator Voltage Registers */
#define AXP22_DCDC1		(0x21)
#define AXP22_DCDC2		(0x22)
#define AXP22_DCDC3		(0x23)
#define AXP22_DCDC4		(0x24)
#define AXP22_DCDC5		(0x25)
#define AXP22_ALDO1		(0x28)
#define AXP22_ALDO2		(0x29)
#define AXP22_ALDO3		(0x2A)
#define AXP22_DLDO1		(0x15)
#define AXP22_DLDO2		(0x16)
#define AXP22_DLDO3		(0x17)
#define AXP22_DLDO4		(0x18)
#define AXP22_ELDO1		(0x19)
#define AXP22_ELDO2		(0x1A)
#define AXP22_ELDO3		(0x1B)
#define AXP22_DC5LDO		(0x1C)
#define AXP22_GPIO0LDO		(0x91)
#define AXP22_GPIO1LDO		(0x93)

/* Unified sub device IDs for AXP */
enum {
	AXP22_ID_DCDC1 = 0,
	AXP22_ID_DCDC2,
	AXP22_ID_DCDC3,
	AXP22_ID_DCDC4,
	AXP22_ID_DCDC5,
	AXP22_ID_RTC,
	AXP22_ID_ALDO1,
	AXP22_ID_ALDO2,
	AXP22_ID_ALDO3,
	AXP22_ID_DLDO1,
	AXP22_ID_DLDO2,
	AXP22_ID_DLDO3,
	AXP22_ID_DLDO4,
	AXP22_ID_ELDO1,
	AXP22_ID_ELDO2,
	AXP22_ID_ELDO3,
	AXP22_ID_LDOIO0,
	AXP22_ID_LDOIO1,
	AXP22_ID_DC1SW,
	AXP22_ID_DC5LDO,
	AXP22_ID_MAX,
};

extern struct axp_dev_info axp22_dev_info;
extern s32 axp22_set_wakeup(void);
extern s32 axp22_suspend(u8 reg, u32 id);

#endif /* __AXP22_POWER_H__ */
