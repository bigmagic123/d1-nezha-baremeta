/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 * axp809
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp809_reg.h>


#define PMU_IC_TYPY_REG (0x3)

#define VDD_SYS_VOL (920)
#define VOL_ON (1)

static axp_contrl_info axp_ctrl_tbl[] = {

	{ "dcdc2", 600, 1540, BOOT_POWER809_DC2OUT_VOL, 0x3f, BOOT_POWER809_OUTPUT_CTL1, 2, 0,
	{ {600, 1540, 20}, } },

	{ "dcdc3", 600, 1860, BOOT_POWER809_DC3OUT_VOL, 0x3f, BOOT_POWER809_OUTPUT_CTL1, 3, 0,
	{ {600, 1860, 20}, } },

	{ "dcdc5", 1000, 2550, BOOT_POWER809_DC5OUT_VOL, 0x1f, BOOT_POWER809_OUTPUT_CTL1, 5, 0,
	{ {1000, 2550, 50}, } },

	{ "aldo3", 700, 3300, BOOT_POWER809_ALDO3OUT_VOL, 0x1f, BOOT_POWER809_OUTPUT_CTL2, 5, 0,
	{ {1000, 2550, 100}, } },

};
#define PMU_POWER_KEY_STATUS BOOT_POWER809_INTSTS5
#define PMU_POWER_KEY_OFFSET 0x3

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
	return axp_ctrl_tbl + i;
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
		if (pmic_bus_read(AXP809_RUNTIME_ADDR, p_item->cfg_reg_addr,
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

		if (pmic_bus_write(AXP809_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP809_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP809_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}


int axp809_probe_power_key(void)
{
	u8 reg_value;

	if (pmic_bus_read(AXP809_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
			  &reg_value)) {
		return -1;
	}
	/* POKLIRQ,POKSIRQ */
	reg_value &= (0x03 << PMU_POWER_KEY_OFFSET);
	if (reg_value) {
		if (pmic_bus_write(AXP809_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
				   reg_value)) {
			return -1;
		}
	}

	return (reg_value >> PMU_POWER_KEY_OFFSET) & 3;
}

int axp809_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdc5", set_vol, 1);
}

int axp809_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("dcdc2", set_vol, 1);
}

int axp809_set_efuse_voltage(int set_vol)
{
	return pmu_set_vol("aldo3", set_vol, 1);
}

int axp809_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc3", set_vol, onoff);
}

int axp809_reg_read(u8 addr, u8 *val)
{
	return pmic_bus_read(AXP809_RUNTIME_ADDR, addr, val);
}

int axp809_reg_write(u8 addr, u8 val)
{
	return pmic_bus_write(AXP809_RUNTIME_ADDR, addr, val);
}



int axp809_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP809_DEVICE_ADDR, AXP809_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP809_RUNTIME_ADDR, PMU_IC_TYPY_REG, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0xCF;
	if (pmu_type == 0x42) {
		/* pmu type AXP809 */
		printf("PMU: AXP809\n");
		axp809_set_sys_voltage(VDD_SYS_VOL, VOL_ON);
		return AXP809_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}
