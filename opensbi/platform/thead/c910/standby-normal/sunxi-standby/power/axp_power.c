// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2015 allwinnertech Ltd.
 * Author: Ming Li <liming@allwinnertech.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include "type.h"
#include "axp_power.h"
#include "../driver/twi/standby_twi.h"

s32 axp_trans_init(struct axp_dev_info *dev_info, u8 saddr, u32 pmu_port)
{
	s32 ret = 0;

	dev_info->pmu_addr = saddr;

	return ret;
}

struct axp_regulator_info *find_info(struct axp_dev_info *dev_info, u32 id)
{
	struct axp_regulator_info *ri = NULL;
	int i = 0;

	for (i = 0; i <= dev_info->pmu_id_max; i++) {
		if (id & (0x1 << i)) {
			ri = dev_info->pmu_regu_table + i;
			break;
		}
	}

	return ri;
}

/* AXP common operations */
s32 axp_byte_read(struct axp_dev_info *dev_info, u8 reg, u8 *reg_val)
{
	s32 ret = 0;

#if defined(CFG_TWI_USED)
	ret = twi_byte_rw(TWI_OP_RD, dev_info->pmu_addr,
			reg, reg_val);
#elif defined(CFG_RSB_USED)
	/* rsb byte rw */
#endif

	return ret;
}

s32 axp_byte_write(struct axp_dev_info *dev_info, u8 reg, u8 *reg_val)
{
	s32 ret = 0;

#if defined(CFG_TWI_USED)
	ret = twi_byte_rw(TWI_OP_WR, dev_info->pmu_addr,
			reg, reg_val);
#elif defined(CFG_RSB_USED)
	/* rsb byte rw */
#endif

	return ret;
}

s32 axp_byte_update(struct axp_dev_info *dev_info, u8 reg, u8 val, u8 mask)
{
	u8 reg_val = 0;
	s32 ret = 0;

	ret = axp_byte_read(dev_info, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | (val & mask);
		ret = axp_byte_write(dev_info, reg, &reg_val);
	}

out:
	return ret;
}

static s32 axp_voltage_get_val(struct axp_regulator_info *info,
			       u32 voltage, u8 *reg_val)
{
	const struct regulator_linear_range *range;
	s32 i;

	if (!info->linear_ranges) {
		*reg_val = (voltage - info->min_uv + info->step1_uv - 1)
			/ info->step1_uv;
		return 0;
	}

	for (i = 0; i < info->n_linear_ranges; i++) {
		int linear_max_uV;

		range = &info->linear_ranges[i];
		linear_max_uV = range->min_uV +
			(range->max_sel - range->min_sel) * range->uV_step;

		if (!(voltage <= linear_max_uV && voltage >= range->min_uV))
			continue;

		/* range->uV_step == 0 means fixed voltage range */
		if (range->uV_step == 0) {
			*reg_val = 0;
		} else {
			*reg_val = (voltage - range->min_uV) / range->uV_step;
		}

		*reg_val += range->min_sel;
		return 0;
	}

	return -1;
}

static s32 axp_val_get_voltage(struct axp_regulator_info *info,
			       u8 reg_val, u32 *voltage)
{
	const struct regulator_linear_range *range;
	s32 i;

	if (!info->linear_ranges) {
		*voltage = info->min_uv + info->step1_uv * reg_val;
		return 0;
	}

	for (i = 0; i < info->n_linear_ranges; i++) {
		if (!(reg_val <= range->max_sel && reg_val >= range->min_sel))
			continue;

		*voltage = (reg_val - range->min_sel) * range->uV_step +
			   range->min_uV;
		return 0;
	}

	return -1;
}

s32 axp_set_volt(struct axp_dev_info *dev_info, u32 id, u32 voltage)
{
	struct axp_regulator_info *info = NULL;
	u8 val, mask, reg_val;
	s32 ret = -1;

	info = find_info(dev_info, id);
	mask = ((1 << info->vol_nbits) - 1);
	if (axp_voltage_get_val(info, voltage, &val))
		return -1;

	ret = axp_byte_read(dev_info, info->vol_reg, &reg_val);
	if (ret)
		return ret;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = axp_byte_write(dev_info, info->vol_reg, &reg_val);
	}

	return ret;
}

s32 axp_get_volt(struct axp_dev_info *dev_info, u32 id)
{
	struct axp_regulator_info *info = NULL;
	u8 val, mask;
	s32 ret, vol;

	info = find_info(dev_info, id);

	ret = axp_byte_read(dev_info, info->vol_reg, &val);
	if (ret)
		return ret;

	mask = ((1 << info->vol_nbits) - 1);
	val = (val & mask);
	if (axp_val_get_voltage(info, val, &vol))
		return -1;

	return vol;
}

s32 axp_set_state(struct axp_dev_info *dev_info, u32 id, u32 state)
{
	struct axp_regulator_info *info = NULL;
	s32 ret = -1;
	u8 reg_val = 0;

	info = find_info(dev_info, id);

	if (state == POWER_VOL_OFF) {
		reg_val = info->disable_val;
		if (!reg_val)
			reg_val = ~info->enable_mask;
	} else {
		reg_val = info->enable_val;
		if (!reg_val)
			reg_val = info->enable_mask;
	}

	return axp_byte_update(dev_info, info->enable_reg,
				reg_val, info->enable_mask);
}

s32 axp_get_state(struct axp_dev_info *dev_info, u32 id)
{
	struct axp_regulator_info *info = NULL;
	s32 ret = -1;
	u8 reg_val;

	info = find_info(dev_info, id);

	ret = axp_byte_read(dev_info, info->enable_reg, &reg_val);
	if (ret)
		return ret;

	if (info->enable_val && reg_val == info->enable_val)
		return POWER_VOL_ON;

	if (info->disable_val && reg_val == info->disable_val)
		return POWER_VOL_OFF;

	if (!info->enable_val && (reg_val & info->enable_mask))
		return POWER_VOL_ON;

	return POWER_VOL_OFF;
}
