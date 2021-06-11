// SPDX-License-Identifier: BSD-2-Clause
#ifndef __AXP_POWER_H__
#define __AXP_POWER_H__

#include <type.h>

typedef enum power_voltage_state {
	POWER_VOL_OFF = 0x0,
	POWER_VOL_ON  = 0x1,
} power_voltage_state_e;

#define AXP(min, max, step1, vreg, nbits, ereg, emask)\
{                                             \
	.min_uv       = (min) * 1000,             \
	.max_uv       = (max) * 1000,             \
	.step1_uv     = (step1) * 1000,           \
	.vol_reg      = vreg,                     \
	.vol_nbits    = (nbits),                  \
	.enable_reg   = ereg,                     \
	.enable_mask  = (emask),                  \
}

#define AXP_FIXED(volt)\
{                                             \
	.min_uv       = (volt) * 1000,             \
	.max_uv       = (volt) * 1000,             \
}

#define AXP_IO(min, max, step1, vreg, nbits, ereg, emask, eval, disval)\
{                                             \
	.min_uv       = (min) * 1000,             \
	.max_uv       = (max) * 1000,             \
	.step1_uv     = (step1) * 1000,           \
	.vol_reg      = vreg,                     \
	.vol_nbits    = (nbits),                  \
	.enable_reg   = ereg,                     \
	.enable_mask  = (emask),                  \
	.enable_val   = eval,                     \
	.disable_val  = disval,                   \
}

#define AXP_RANGES(_ranges, vreg, nbits, ereg, emask)\
{                                             \
	.vol_reg      = vreg,                     \
	.vol_nbits    = (nbits),                  \
	.enable_reg   = ereg,                     \
	.enable_mask  = (emask),                  \
	.linear_ranges  = (_ranges),              \
	.n_linear_ranges = ARRAY_SIZE(_ranges),   \
}

#define AXP_SW(ereg, emask)\
{                                             \
	.enable_reg   = ereg,                     \
	.enable_mask  = (emask),                  \
}

#define AXP_SEL(min, max, vreg, nbits, ereg, emask, table_name)\
{                                                    \
	.min_uv      = (min) * 1000,                     \
	.max_uv      = (max) * 1000,                     \
	.vol_reg     = vreg,                             \
	.vol_nbits   = (nbits),                          \
	.enable_reg  = ereg,                             \
	.enable_mask = (emask),                          \
	.vtable      = (int *)&table_name##_table,       \
}

/* Initialize struct regulator_linear_range */
#define REGULATOR_LINEAR_RANGE(_min_uV, _min_sel, _max_sel, _step_uV)   \
{                                                                       \
	.min_uV         = _min_uV,                                      \
	.min_sel        = _min_sel,                                     \
	.max_sel        = _max_sel,                                     \
	.uV_step        = _step_uV,                                     \
}

struct regulator_linear_range {
	unsigned int min_uV;
	unsigned int min_sel;
	unsigned int max_sel;
	unsigned int uV_step;
};

struct axp_regulator_info {
	int min_uv;
	int max_uv;
	int step1_uv;
	int vol_reg;
	int vol_nbits;
	int enable_reg;
	int enable_mask;
	int enable_val;
	int disable_val;
	const struct regulator_linear_range *linear_ranges;
	int n_linear_ranges;
	int *vtable;
};

struct axp_dev_info {
	u32 pmu_addr;
	u32 pmu_para;
	u32 pmu_id_max;
	struct axp_regulator_info *pmu_regu_table;
};

extern struct axp_regulator_info *find_info(struct axp_dev_info *dev_info,
				u32 id);
extern s32 axp_trans_init(struct axp_dev_info *dev_info, u8 saddr, u32 pmu_port);
extern s32 axp_set_volt(struct axp_dev_info *dev_info, u32 id, u32 voltage);
extern s32 axp_get_volt(struct axp_dev_info *dev_info, u32 id);
extern s32 axp_set_state(struct axp_dev_info *dev_info, u32 id, u32 state);
extern s32 axp_get_state(struct axp_dev_info *dev_info, u32 id);
extern s32 axp_byte_read(struct axp_dev_info *dev_info, u8 reg, u8 *reg_val);
extern s32 axp_byte_write(struct axp_dev_info *dev_info, u8 reg, u8 *reg_val);
extern s32 axp_byte_update(struct axp_dev_info *dev_info, u8 reg, u8 val, u8 mask);

#endif
