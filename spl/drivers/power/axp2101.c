/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp2101_reg.h>
#include <private_boot0.h>

static int pmu_set_vol(char *name, int set_vol, int onoff);
#define VDD_SYS_VOL (920)
#define VOL_ON (1)

static axp_contrl_info axp_ctrl_tbl[] = {
	{ "dcdc2", 500, 1540, AXP2101_DC2OUT_VOL, 0x7f, AXP2101_OUTPUT_CTL0, 1, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc3", 500, 3400, AXP2101_DC3OUT_VOL, 0x7f, AXP2101_OUTPUT_CTL0, 2, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, {1600, 3400, 100}, } },

	{ "dcdc4", 500, 1840, AXP2101_DC4OUT_VOL, 0x7f, AXP2101_OUTPUT_CTL0, 3, 0,
	{ {500, 1200, 10}, {1220, 1840, 20}, } },

	{ "bldo1", 500, 3500, AXP2101_BLDO1OUT_VOL, 0x1f, AXP2101_OUTPUT_CTL2, 4, 0,
	{ {500, 3500, 100}, } },

};
#define PMU_POWER_KEY_STATUS AXP2101_INTSTS1
#define PMU_POWER_KEY_OFFSET 0x2

static inline void disable_dcdc_pfm_mode(void)
{
	u8 val;

	pmic_bus_read(AXP2101_RUNTIME_ADDR, 0x80, &val);
	val |= (0x01 << 3); /*dcdc4 for gpu pwm mode*/
	val |= (0x01 << 4); /*dcdc5 for dram pwm mode*/
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0x80, val);

	/* disable dcm mode for GPU stability Vdrop issue*/
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0xff, 0x0);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0xf4, 0x6);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0xf2, 0x4);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0xf5, 0x4);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0xff, 0x1);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0x12, 0x40);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, 0xff, 0x0);
}

int axp2101_probe_power_key(void)
{
	u8 reg_value;

	if (pmic_bus_read(AXP2101_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
			  &reg_value)) {
		return -1;
	}
	/* POKLIRQ,POKSIRQ */
	reg_value &= (0x03 << PMU_POWER_KEY_OFFSET);
	if (reg_value) {
		if (pmic_bus_write(AXP2101_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
				   reg_value)) {
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
		if (pmic_bus_read(AXP2101_RUNTIME_ADDR, p_item->cfg_reg_addr,
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

		if (pmic_bus_write(AXP2101_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP2101_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP2101_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}

int axp2101_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdc4", set_vol, 1);
}

int axp2101_set_efuse_voltage(int set_vol)
{
	return pmu_set_vol("bldo1", set_vol, 1);
}

int axp2101_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("dcdc2", set_vol, 1);
}

int axp2101_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc3", set_vol, onoff);
}

/*
 * pmu_type : 0x47 is the first version
 *            0x4a is the second version
 */
static int axp2101_set_necessary_reg(int pmu_type)
{
	u8 reg_value;

	/* limit charge current to 300mA */
	reg_value = 0x9;
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_CHARGE1, reg_value);

	/* limit run current to 2A */
	reg_value = 0x5;
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_VBUS_CUR_SET, reg_value);

	/*enable vbus adc channel*/
	if (pmu_type != AXP2101_CHIP_ID_B) {
		reg_value = 0x40;
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_BAT_AVERVOL_H6, reg_value);
	}

	/*set dcdc1 pwm mode*/
	pmic_bus_read(AXP2101_RUNTIME_ADDR, AXP2101_OUTPUT_CTL1, &reg_value);
	reg_value |= (1 << 2);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_OUTPUT_CTL1, reg_value);

	/*pmu disable soften3 signal*/
	if (pmu_type != AXP2101_CHIP_ID_B) {
		reg_value = 0x00;
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_TWI_ADDR_EXT, reg_value);
		reg_value = 0x06;
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_EFUS_OP_CFG, reg_value);
		reg_value = 0x04;
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_EFREQ_CTRL, reg_value);
		reg_value = 0x01;
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_TWI_ADDR_EXT, reg_value);
		reg_value = 0x30;
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_SELLP_CFG, reg_value);
		reg_value = 0x00;
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_TWI_ADDR_EXT, reg_value);
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_EFREQ_CTRL, reg_value);
		pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_EFUS_OP_CFG, reg_value);
	}

	/*pmu set vsys min*/
	pmic_bus_read(AXP2101_RUNTIME_ADDR, AXP2101_VSYS_MIN, &reg_value);
	reg_value &= ~(0x7 << 4);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_VSYS_MIN, reg_value);

	/*pmu set vimdpm cfg*/
	pmic_bus_read(AXP2101_RUNTIME_ADDR, AXP2101_VBUS_VOL_SET, &reg_value);
	reg_value &= ~(0xf << 0);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_VBUS_VOL_SET, reg_value);

	/*pmu reset enable*/
	pmic_bus_read(AXP2101_RUNTIME_ADDR, AXP2101_OFF_CTL, &reg_value);
	reg_value |= (3 << 2);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_OFF_CTL, reg_value);

	/*pmu pwroff enable*/
	pmic_bus_read(AXP2101_RUNTIME_ADDR, AXP2101_PWEON_PWEOFF_EN, &reg_value);
	reg_value |= (1 << 1);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_PWEON_PWEOFF_EN, reg_value);

	/*pmu dcdc1 pwroff enable*/
	pmic_bus_read(AXP2101_RUNTIME_ADDR, AXP2101_DCDC_PWEOFF_EN, &reg_value);
	reg_value &= ~(1 << 0);
	pmic_bus_write(AXP2101_RUNTIME_ADDR, AXP2101_DCDC_PWEOFF_EN, reg_value);

	return 0;
}

int axp2101_reg_read(u8 addr, u8 *val)
{
	return pmic_bus_read(AXP2101_RUNTIME_ADDR, addr, val);
}

int axp2101_reg_write(u8 addr, u8 val)
{
	return pmic_bus_write(AXP2101_RUNTIME_ADDR, addr, val);
}


int axp2101_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP2101_DEVICE_ADDR, AXP2101_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP2101_RUNTIME_ADDR, AXP2101_VERSION, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0xCF;
	if (pmu_type == AXP2101_CHIP_ID || pmu_type == AXP2101_CHIP_ID_B) {
		/* pmu type AXP21 */
		printf("PMU: AXP21\n");
		axp2101_set_necessary_reg(pmu_type);
		axp2101_set_sys_voltage(VDD_SYS_VOL, VOL_ON);
		return AXP2101_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}
