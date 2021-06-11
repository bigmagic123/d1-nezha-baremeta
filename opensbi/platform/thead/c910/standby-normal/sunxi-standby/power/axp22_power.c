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

#include <stddef.h>
#include "type.h"
#include "axp_power.h"
#include "axp22_power.h"

#ifdef CFG_AXP22X_USED

static struct axp_regulator_info axp22_regulator_info[] = {
	AXP(1600, 3400, 100, AXP22_DCDC1, 5, AXP22_LDO_DC_EN1, BIT(1)),
	AXP(600, 1540, 20, AXP22_DCDC2, 6, AXP22_LDO_DC_EN1, BIT(2)),
	AXP(600, 1860, 20, AXP22_DCDC3, 6, AXP22_LDO_DC_EN1, BIT(3)),
	AXP(600, 1540, 20, AXP22_DCDC4, 6, AXP22_LDO_DC_EN1, BIT(4)),
	AXP(1000, 2550, 50, AXP22_DCDC5, 5, AXP22_LDO_DC_EN1, BIT(5)),
	AXP(3000, 3000, 0, 0, 0, 0, 0), /* always on */
	AXP(700, 3300, 100, AXP22_ALDO1, 5, AXP22_LDO_DC_EN1, BIT(6)),
	AXP(700, 3300, 100, AXP22_ALDO2, 5, AXP22_LDO_DC_EN1, BIT(7)),
	AXP(700, 3300, 100, AXP22_ALDO3, 5, AXP22_LDO_DC_EN3, BIT(7)),
	AXP(700, 3300, 100, AXP22_DLDO1, 5, AXP22_LDO_DC_EN2, BIT(3)),
	AXP(700, 3300, 100, AXP22_DLDO2, 5, AXP22_LDO_DC_EN2, BIT(4)),
	AXP(700, 3300, 100, AXP22_DLDO3, 5, AXP22_LDO_DC_EN2, BIT(5)),
	AXP(700, 3300, 100, AXP22_DLDO4, 5, AXP22_LDO_DC_EN2, BIT(6)),
	AXP(700, 3300, 100, AXP22_ELDO1, 5, AXP22_LDO_DC_EN2, BIT(0)),
	AXP(700, 3300, 100, AXP22_ELDO2, 5, AXP22_LDO_DC_EN2, BIT(1)),
	AXP(700, 3300, 100, AXP22_ELDO3, 5, AXP22_LDO_DC_EN2, BIT(2)),
	AXP_IO(700, 3300, 100, AXP22_GPIO0LDO, 5,
	       AXP22_GPIO0_CTL, 0x07, 0x07, 0x03),
	AXP_IO(700, 3300, 100, AXP22_GPIO1LDO, 5,
	       AXP22_GPIO1_CTL, 0x07, 0x07, 0x03),
	AXP_SW(AXP22_LDO_DC_EN2, BIT(7)), /* dc1sw */
	AXP(700, 1400, 100, AXP22_DC5LDO, 3, AXP22_LDO_DC_EN1, BIT(0)),
};

struct axp_dev_info axp22_dev_info = {
	.pmu_id_max     = AXP22_ID_MAX,
	.pmu_regu_table = axp22_regulator_info,
};

s32 axp22_set_wakeup(void)
{
	u8 reg_val;
	s32 ret = 0;

	ret = axp_byte_read(&axp22_dev_info, 0x31, &reg_val);
	if (ret)
		goto out;

	reg_val |= 0x1 << 3;
	reg_val &= ~(0x1 << 4);
	reg_val |= 0x1 << 7;

	ret = axp_byte_write(&axp22_dev_info, 0x31, &reg_val);

out:
	return ret;
}

s32 axp22_suspend(u8 reg, u32 id)
{
	struct axp_regulator_info *info = NULL;
	u8 enable_mask;
	u8 reg_val = 0xff;
	u32 i;

	for (i = 0; i < AXP22_ID_MAX; i++) {
		if (0 == (id & 1 << i))
			continue;

		info = find_info(&axp22_dev_info, (0x1 << i));
		if (reg != info->enable_reg)
			continue;

		enable_mask |= info->enable_mask;

		if (info->disable_val) {
			reg_val = (reg_val & ~info->enable_mask) |
				   info->disable_val;
		} else {
			reg_val &= ~info->enable_mask;
		}
	}

	return axp_byte_update(&axp22_dev_info, reg, reg_val, enable_mask);
}

#endif /* CFG_AXP22X_USED */
