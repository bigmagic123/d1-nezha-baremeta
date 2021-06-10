/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp858_reg.h>

static int pmu_set_vol(char *name, int set_vol, int onoff);

static axp_contrl_info axp_ctrl_tbl[] = {
	{ "dcdc2", 500, 1540, PMU_DC2OUT_VOL, 0x7f, PMU_ONOFF_CTL1, 1, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc3", 500, 1540, PMU_DC3OUT_VOL, 0x7f, PMU_ONOFF_CTL1, 2, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc4", 500, 1540, PMU_DC4OUT_VOL, 0x7f, PMU_ONOFF_CTL1, 3, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc5", 800, 1840, PMU_DC5OUT_VOL, 0x7f, PMU_ONOFF_CTL1, 4, 0,
	{ {800, 1120, 10}, {1140, 1840, 20}, } },

	{ "aldo2", 700, 3300, PMU_ALDO2_VOL, 0x1f, PMU_ONOFF_CTL2, 1, 0,
	{ {700, 3300, 100}, } },

	{ "aldo5", 700, 3300, PMU_ALDO5_VOL, 0x1f, PMU_ONOFF_CTL2, 4, 0,
	{ {700, 3300, 100}, } },

};
#define PMU_POWER_KEY_STATUS 0x4c
#define PMU_POWER_KEY_OFFSET 0x3
#if 0
static int pwrok_restart_enable(void)
{
	u8 reg_val = 0;
	if (pmic_bus_read(AXP858_RUNTIME_ADDR, PMU_POWER_DISABLE_DOWN, &reg_val)) {
		return -1;
	}
	/* PWROK drive low restart function enable  */
	/* for watchdog reset */
	reg_val |= (0x1U << 6);
	if (pmic_bus_write(AXP858_RUNTIME_ADDR, PMU_POWER_DISABLE_DOWN, reg_val)) {
		return -1;
	}
	return 0;
}
#endif
static inline void disable_dcdc_pfm_mode(void)
{
	u8 val;

	pmic_bus_read(AXP858_RUNTIME_ADDR, 0x80, &val);
	val |= (0x01 << 3); /*dcdc4 for gpu pwm mode*/
	val |= (0x01 << 4); /*dcdc5 for dram pwm mode*/
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0x80, val);

	/* disable dcm mode for GPU stability Vdrop issue*/
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0xff, 0x0);
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0xf4, 0x6);
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0xf2, 0x4);
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0xf5, 0x4);
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0xff, 0x1);
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0x12, 0x40);
	pmic_bus_write(AXP858_RUNTIME_ADDR, 0xff, 0x0);
}

int axp858_probe_power_key(void)
{
	u8 reg_value;

	if (pmic_bus_read(AXP858_RUNTIME_ADDR, PMU_POWER_KEY_STATUS, &reg_value)) {
		return -1;
	}
	/* POKLIRQ,POKSIRQ */
	reg_value &= (0x03 << PMU_POWER_KEY_OFFSET);
	if (reg_value) {
		if (pmic_bus_write(AXP858_RUNTIME_ADDR, PMU_POWER_KEY_STATUS, reg_value)) {
			return -1;
		}
	}

	return (reg_value >> PMU_POWER_KEY_OFFSET) & 3;
}

static axp_contrl_info *get_ctrl_info_from_tbl(char *name)
{
	int i    = 0;
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

static int pmu_set_vol(char *name, int set_vol, int onoff)
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
		if (pmic_bus_read(AXP858_RUNTIME_ADDR, p_item->cfg_reg_addr,
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

		if (pmic_bus_write(AXP858_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP858_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP858_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}

int axp858_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdc5", set_vol, 1);
}

int axp858_set_ddr4_2v5_voltage(int set_vol)
{
	return pmu_set_vol("aldo5", set_vol, 1);
}

int axp858_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("dcdc2", set_vol, 1);
}

int axp858_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc3", set_vol, onoff);
}

int axp858_set_sys_voltage_ext(char *name, int set_vol, int onoff)
{
	return pmu_set_vol(name, set_vol, onoff);
}

static int axp858_necessary_reg_enable(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP858_RUNTIME_ADDR, PMU_DCDC_MODE_CTL1, &reg_value)) {
		return -1;
	}
	reg_value |= (1 << 0);
	if (pmic_bus_write(AXP858_RUNTIME_ADDR, PMU_DCDC_MODE_CTL1, reg_value)) {
		return -1;
	}
	return 0;
}

int axp858_reg_read(u8 addr, u8 *val)
{
	return pmic_bus_read(AXP858_RUNTIME_ADDR, addr, val);
}

int axp858_reg_write(u8 addr, u8 val)
{
	return pmic_bus_write(AXP858_RUNTIME_ADDR, addr, val);
}


int axp858_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP858_DEVICE_ADDR, AXP858_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP858_RUNTIME_ADDR, PMU_IC_TYPE, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0xCF;
	if (pmu_type == 0x44) {
		/* pmu type AXP858 */
		printf("PMU: AXP858\n");
		/*pwrok_restart_enable();*/
		/*disable_dcdc_pfm_mode();*/
		axp858_necessary_reg_enable();
		return AXP858_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}
