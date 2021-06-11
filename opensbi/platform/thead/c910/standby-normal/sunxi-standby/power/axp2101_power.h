// SPDX-License-Identifier: BSD-2-Clause
/*
 * For: AXP2101		AW1819
 *
 * Copyright (C) 2019 allwinnertech Ltd.
 * Author: frank <frank@allwinnertech.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __AXP2101_POWER_H__
#define __AXP2101_POWER_H__

/* AXP2101 Sleep & Wakeup Registers */
#define AXP2101_SLEEP_CFG	(0x26)

/* AXP2101 Regulator Enable Registers */
#define AXP2101_DCDC_CFG0	(0x80)
#define AXP2101_LDO_EN_CFG0	(0x90)
#define AXP2101_LDO_EN_CFG1	(0x91)

/* AXP2101 Regulator Voltage Registers */
#define AXP2101_DCDC1		(0x82)
#define AXP2101_DCDC2		(0x83)
#define AXP2101_DCDC3		(0x84)
#define AXP2101_DCDC4		(0x85)
#define AXP2101_DCDC5		(0x86)
#define AXP2101_ALDO1		(0x92)
#define AXP2101_ALDO2		(0x93)
#define AXP2101_ALDO3		(0x94)
#define AXP2101_ALDO4		(0x95)
#define AXP2101_BLDO1		(0x96)
#define AXP2101_BLDO2		(0x97)
#define AXP2101_CPUSLDO		(0x98)
#define AXP2101_DLDO1		(0x99)
#define AXP2101_DLDO2		(0x9A)

/* Unified sub device IDs for AXP */
enum {
	AXP2101_ID_DCDC1 = 0,
	AXP2101_ID_DCDC2,
	AXP2101_ID_DCDC3,
	AXP2101_ID_DCDC4,
	AXP2101_ID_DCDC5,
	AXP2101_ID_RTCLDO,
	AXP2101_ID_RTCLDO1,
	AXP2101_ID_ALDO1,
	AXP2101_ID_ALDO2,
	AXP2101_ID_ALDO3,
	AXP2101_ID_ALDO4,
	AXP2101_ID_BLDO1,
	AXP2101_ID_BLDO2,
	AXP2101_ID_DLDO1,
	AXP2101_ID_DLDO2,
	AXP2101_ID_CPUSLDO,
	AXP2101_ID_MAX,
};

extern struct axp_dev_info axp2101_dev_info;
extern s32 axp2101_set_wakeup(void);
extern s32 axp2101_suspend(u8 reg, u32 id);

#endif /* __AXP2101_POWER_H__ */
