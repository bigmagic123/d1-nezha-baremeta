/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp152_reg.h>
#include <private_boot0.h>

__attribute__((section(".data"))) axp_contrl_info axp_ctrl_tbl[] = {
	{ "dcdc1", 1700, 3500, AXP152_DC1OUT_VOL, 0x0f, AXP152_OUTPUT_CTL, 7, 0,
	{ {1700, 2100, 100}, {2400, 2800, 100}, {3000, 3500, 100} } },

	{ "dcdc2", 700, 2275, AXP152_DC2OUT_VOL, 0x3f, AXP152_OUTPUT_CTL, 6, 0,
	{ {700, 2275, 25}, } },

	{ "dcdc3", 700, 3500, AXP152_DC3OUT_VOL, 0x3f, AXP152_OUTPUT_CTL, 5, 0,
	{ {700, 3500, 50}, } },

	{ "dcdc4", 700, 3500, AXP152_DC4OUT_VOL, 0x7f, AXP152_OUTPUT_CTL, 4, 0,
	{ {700, 3500, 25}, } },

	{ "aldo1", 1200, 3300, AXP152_ALDO12OUT_VOL, 0xf0, AXP152_OUTPUT_CTL, 3, 4,
	{ {1200, 2000, 100}, {2500, 2700, 200}, {2800, 3000, 200}, {3100, 3300, 100} } },

	{ "aldo2", 1200, 3300, AXP152_ALDO12OUT_VOL, 0x0f, AXP152_OUTPUT_CTL, 2, 0,
	{ {1200, 2000, 100}, {2500, 2700, 200}, {2800, 3000, 200}, {3100, 3300, 100} } },

	{ "dldo1", 700, 3500, AXP152_DLDO1OUT_VOL, 0x1f, AXP152_OUTPUT_CTL, 1, 0,
	{ {700, 3500, 100}, } },

	{ "dldo2", 700, 3500, AXP152_DLDO2OUT_VOL, 0x1f, AXP152_OUTPUT_CTL, 0, 0,
	{ {700, 3500, 100}, } },

	{ "ldo0", 2500, 5000, AXP152_LDO0_VOL, 0x30, AXP152_LDO0_VOL, 7, 4,
	{ {2500, 2800, 300, 1}, {3300, 5000, 1700, 1}, }, },

	{ "gpio2ldo", 1800, 3300, AXP152_GPIO2_LDO_MOD, 0x0f, AXP152_GPIO2_CTL, 7, 0,
	{ {1800, 3300, 100}, } },

};

static int pmu_set_vol(char *name, int set_vol, int onoff);

#define PMU_POWER_KEY_STATUS AXP152_INTSTS1
#define PMU_POWER_KEY_OFFSET 0x2

int axp152_probe_power_key(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_INTSTS2, &reg_value)) {
		return -1;
	}
	reg_value &= (0x03);
	if (reg_value) {
		if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_INTSTS2,
				   reg_value | (0x01 << 1))) {
			return -1;
		}
	}
	return reg_value & 3;
}

static axp_contrl_info *get_ctrl_info_from_tbl(char *name)
{
	int i	 = 0;
	int size = ARRAY_SIZE(axp_ctrl_tbl);
	for (i = 0; i < size; i++) {
		if (!strncmp(name, axp_ctrl_tbl[i].name,
			     strlen(axp_ctrl_tbl[i].name))) {
			break;
		}
	}
	if (i >= size) {
		return NULL;
	}
	return (axp_ctrl_tbl + i);
}

static int pmu_axp152_set_vol(char *name, int set_vol, int onoff)
{
	u8 reg_value, i;
	axp_contrl_info *p_item = NULL;
	u8 base_step		= 0;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	if ((set_vol > 0) && (p_item->min_vol)) {
		if (set_vol < p_item->min_vol) {
			set_vol = p_item->min_vol;
		} else if (set_vol > p_item->max_vol) {
			set_vol = p_item->max_vol;
		}
		if (pmic_bus_read(AXP152_RUNTIME_ADDR, p_item->cfg_reg_addr,
				  &reg_value)) {
			return -1;
		}

		reg_value &= ~p_item->cfg_reg_mask;

		for (i = 0; p_item->axp_step_tbl[i].step_max_vol != 0; i++) {
			if ((set_vol > p_item->axp_step_tbl[i].step_max_vol) &&
				(set_vol < p_item->axp_step_tbl[i+1].step_min_vol)) {
				set_vol = p_item->axp_step_tbl[i].step_max_vol;
			}
			if (p_item->axp_step_tbl[i].step_max_vol >= set_vol) {
				reg_value |= ((base_step + ((set_vol - p_item->axp_step_tbl[i].step_min_vol)/
					p_item->axp_step_tbl[i].step_val)) << p_item->reg_addr_offest);
				if (p_item->axp_step_tbl[i].regation) {
					u8 reg_value_temp = (~reg_value & p_item->cfg_reg_mask);
					reg_value &= ~p_item->cfg_reg_mask;
					reg_value |= reg_value_temp;
				}
				break;
			} else {
				base_step += ((p_item->axp_step_tbl[i].step_max_vol -
					p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
					p_item->axp_step_tbl[i].step_val);
			}
		}

		if (pmic_bus_write(AXP152_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP152_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}

static int pmu_get_vol(char *name)
{

	u8 reg_value, i;
	axp_contrl_info *p_item = NULL;
	u8 base_step1 = 0;
	u8 base_step2 = 0;
	int vol;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	if (pmic_bus_read(AXP152_RUNTIME_ADDR, p_item->ctrl_reg_addr,
		&reg_value)) {
		return -1;
	}
	if (!(reg_value & (0x01 << p_item->ctrl_bit_ofs))) {
		return 0;
	}

	if (pmic_bus_read(AXP152_RUNTIME_ADDR, p_item->cfg_reg_addr,
		&reg_value)) {
		return -1;
	}
	reg_value &= p_item->cfg_reg_mask;
	reg_value >>= p_item->reg_addr_offest;
	for (i = 0; p_item->axp_step_tbl[i].step_max_vol != 0; i++) {
		base_step1 += ((p_item->axp_step_tbl[i].step_max_vol -
				p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
				p_item->axp_step_tbl[i].step_val);
		if (p_item->axp_step_tbl[i].regation)
				reg_value = (~reg_value & (p_item->cfg_reg_mask >> p_item->reg_addr_offest));
		if (reg_value < base_step1) {
			vol =  (reg_value - base_step2) * p_item->axp_step_tbl[i].step_val +
					p_item->axp_step_tbl[i].step_min_vol;
			return vol;
		}
		if (p_item->axp_step_tbl[i].regation)
			reg_value = (~reg_value & (p_item->cfg_reg_mask >> p_item->reg_addr_offest));
		base_step2 += ((p_item->axp_step_tbl[i].step_max_vol -
				p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
				p_item->axp_step_tbl[i].step_val);
	}

	return -1;
}


static int pmu_set_vol(char *name, int set_vol, int onoff)
{
	int i, temp_vol, src_vol = pmu_get_vol(name);
	u32 step_voltage = 0xffff;
	axp_contrl_info *p_item = NULL;
	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	for (i = 0; p_item->axp_step_tbl[i].step_val != 0; i++) {
		step_voltage = min(step_voltage, p_item->axp_step_tbl[i].step_val);
	}

	if (step_voltage == 0xffff)
		return -1;

	if (src_vol > set_vol) {
		for (temp_vol = src_vol; temp_vol >= set_vol; temp_vol -= step_voltage) {
			if (pmu_axp152_set_vol(name, temp_vol, onoff))
				return -1;
		}
		udelay(step_voltage*10/25);
	} else if (src_vol < set_vol) {
		for (temp_vol = src_vol; temp_vol <= set_vol; temp_vol += step_voltage) {
			if (pmu_axp152_set_vol(name, temp_vol, onoff))
				return -1;
		}
		udelay(step_voltage*10/25);
	}
	return 0;
}

int axp152_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdc3", set_vol, 1);
}

int axp152_set_efuse_voltage(int set_vol)
{
	return pmu_set_vol("aldo1", set_vol, 1);
}

int axp152_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("dcdc2", set_vol, 1);
}

int axp152_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc1", set_vol, onoff);
}

int axp152_necessary_reg_enable(void)
{
	u8 reg_value;
	reg_value = 0;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_VOFF_SET,
			   reg_value)) {
		return -1;
	}

	reg_value = 0xd;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_POK_SET,
			   reg_value)) {
		return -1;
	}

	reg_value = 0xf;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_DCDC_MODESET,
			   reg_value)) {
		return -1;
	}

	reg_value = 0xdd;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_VOUT_MONITOR,
		   reg_value)) {
		return -1;
	}

	reg_value = 0x85;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_HOTOVER_CTL,
		   reg_value)) {
		return -1;
	}

	reg_value = 0x03;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_GPIO1_CTL,
		   reg_value)) {
		return -1;
	}

	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_HOTOVER_CTL, &reg_value)) {
		return -1;
	}
	reg_value &= ~(0x3);
	reg_value |= 0x2;
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_HOTOVER_CTL, reg_value)) {
		return -1;
	}

	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_DC2OUT_DVM, &reg_value)) {
		return -1;
	}
	reg_value |= (0x5);
	if (pmic_bus_write(AXP152_RUNTIME_ADDR, AXP152_DC2OUT_DVM, reg_value)) {
		return -1;
	}
	return 0;
}

int axp152_reg_read(u8 addr, u8 *val)
{
	return pmic_bus_read(AXP152_RUNTIME_ADDR, addr, val);
}

int axp152_reg_write(u8 addr, u8 val)
{
	return pmic_bus_write(AXP152_RUNTIME_ADDR, addr, val);
}

int axp152_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP152_DEVICE_ADDR, AXP152_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP152_RUNTIME_ADDR, AXP152_VERSION, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0x0F;
	if (pmu_type == AXP152_CHIP_ID) {
		/* pmu type AXP152 */
		printf("PMU: AXP152\n");
	axp152_necessary_reg_enable();
	/*only use h616 perf1_axp152*/
	if (power_mode == 3) {
		pmu_set_vol("ldo0", 3300, 1);
		udelay(1);
		pmu_set_vol("dldo1", 3300, 0);
	}
		return AXP152_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}
