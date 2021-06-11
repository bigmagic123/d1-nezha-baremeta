// SPDX-License-Identifier: BSD-2-Clause
/*
 * For: AXP2101		AW1819
 *
 * Copyright (C) 2019 allwinnertech Ltd.
 * Author: frank <frank@allwinnertech.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include "type.h"
#include "axp_power.h"
#include "axp2101_power.h"

#ifdef CFG_AXP2101_USED

static const struct regulator_linear_range dcdc2_ranges[] = {
	REGULATOR_LINEAR_RANGE(500000, 0x0, 0x46, 10000),
	REGULATOR_LINEAR_RANGE(1220000, 0x47, 0x57, 20000),
};

static const struct regulator_linear_range dcdc3_ranges[] = {
	REGULATOR_LINEAR_RANGE(500000, 0x0, 0x46, 10000),
	REGULATOR_LINEAR_RANGE(1220000, 0x47, 0x57, 20000),
	REGULATOR_LINEAR_RANGE(1600000, 0x58, 0x6a, 100000),
};

static const struct regulator_linear_range dcdc4_ranges[] = {
	REGULATOR_LINEAR_RANGE(500000, 0x0, 0x46, 10000),
	REGULATOR_LINEAR_RANGE(1220000, 0x47, 0x66, 20000),
};

static struct axp_regulator_info axp2101_regulator_info[] = {
	AXP(1500, 3400, 100, AXP2101_DCDC1, 5, AXP2101_DCDC_CFG0, BIT(0)),
	AXP_RANGES(dcdc2_ranges, AXP2101_DCDC2, 7, AXP2101_DCDC_CFG0, BIT(1)),
	AXP_RANGES(dcdc3_ranges, AXP2101_DCDC3, 7, AXP2101_DCDC_CFG0, BIT(2)),
	AXP_RANGES(dcdc4_ranges, AXP2101_DCDC4, 7, AXP2101_DCDC_CFG0, BIT(3)),
	AXP(1400, 3700, 100, AXP2101_DCDC5, 5, AXP2101_DCDC_CFG0, BIT(4)),
	AXP_FIXED(1800),
	AXP_FIXED(1800),
	AXP(500, 3500, 100, AXP2101_ALDO1, 5, AXP2101_LDO_EN_CFG0, BIT(0)),
	AXP(500, 3500, 100, AXP2101_ALDO2, 5, AXP2101_LDO_EN_CFG0, BIT(1)),
	AXP(500, 3500, 100, AXP2101_ALDO3, 5, AXP2101_LDO_EN_CFG0, BIT(2)),
	AXP(500, 3500, 100, AXP2101_ALDO4, 5, AXP2101_LDO_EN_CFG0, BIT(3)),
	AXP(500, 3500, 100, AXP2101_BLDO1, 5, AXP2101_LDO_EN_CFG0, BIT(4)),
	AXP(500, 3500, 100, AXP2101_BLDO2, 5, AXP2101_LDO_EN_CFG0, BIT(5)),
	AXP(500, 3500, 100, AXP2101_DLDO1, 5, AXP2101_LDO_EN_CFG0, BIT(7)),
	AXP(500, 3500, 100, AXP2101_DLDO2, 5, AXP2101_LDO_EN_CFG1, BIT(0)),
	AXP(500, 3500, 100, AXP2101_CPUSLDO, 5, AXP2101_LDO_EN_CFG0, BIT(6)),
};

struct axp_dev_info axp2101_dev_info = {
	.pmu_id_max     = AXP2101_ID_MAX,
	.pmu_regu_table = axp2101_regulator_info,
};

s32 axp2101_set_wakeup(void)
{
	u8 reg_val = 0x1d;

	return  axp_byte_write(&axp2101_dev_info, AXP2101_SLEEP_CFG, &reg_val);
}

s32 axp2101_suspend(u8 reg, u32 id)
{
	struct axp_regulator_info *info = NULL;
	u8 enable_mask = 0;
	u8 reg_val = 0xf;
	u32 i;

	for (i = 0; i < AXP2101_ID_MAX; i++) {
		if (0 == (id & BIT(i)))
			continue;

		info = find_info(&axp2101_dev_info, (0x1 << i));
		if (reg == info->enable_reg) {
			reg_val &= ~info->enable_mask;
			enable_mask |= info->enable_mask;
		}
	}

	return axp_byte_update(&axp2101_dev_info, reg, reg_val, enable_mask);
}

#endif /* CFG_AXP2101_USED */
