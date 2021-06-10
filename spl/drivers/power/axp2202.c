/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp2202_reg.h>
#include <private_boot0.h>

static int pmu_set_vol(char *name, int set_vol, int onoff);
#define VDD_SYS_VOL (920)
#define VOL_ON (1)

static axp_contrl_info axp_ctrl_tbl[] = {
	{ "dcdc1", 500, 1540, AXP2202_DC1OUT_VOL, 0x7f, AXP2202_OUTPUT_CTL0, 0, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, } },

	{ "dcdc2", 500, 3400, AXP2202_DC2OUT_VOL, 0x7f, AXP2202_OUTPUT_CTL0, 1, 0,
	{ {500, 1200, 10}, {1220, 1540, 20}, {1600, 3400, 100},} },

	{ "dcdc3", 500, 1840, AXP2202_DC3OUT_VOL, 0x7f, AXP2202_OUTPUT_CTL0, 2, 0,
	{ {500, 1200, 10}, {1220, 1840, 20}, } },

	{ "aldo1", 500, 3500, AXP2202_ALDO1OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 0, 0,
	{ {500, 3500, 100}, } },

	{ "aldo2", 500, 3500, AXP2202_ALDO2OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 1, 0,
	{ {500, 3500, 100}, } },

	{ "aldo3", 500, 3500, AXP2202_ALDO3OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 2, 0,
	{ {500, 3500, 100}, } },

	{ "aldo4", 500, 3500, AXP2202_ALDO4OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 3, 0,
	{ {500, 3500, 100}, } },

	{ "bldo1", 500, 3500, AXP2202_BLDO1OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 4, 0,
	{ {500, 3500, 100}, } },

	{ "bldo2", 500, 3500, AXP2202_BLDO2OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 5, 0,
	{ {500, 3500, 100}, } },

	{ "bldo3", 500, 3500, AXP2202_BLDO3OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 6, 0,
	{ {500, 3500, 100}, } },

	{ "bldo4", 500, 3500, AXP2202_BLDO4OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL2, 7, 0,
	{ {500, 3500, 100}, } },

	{ "cldo1", 500, 3500, AXP2202_CLDO1OUT_VOL, 0x1f, AXP2202_OUTPUT_CTL3, 0, 0,
	{ {500, 3500, 100}, } },


};
#define PMU_POWER_KEY_STATUS AXP2202_INTSTS1
#define PMU_POWER_KEY_OFFSET 0x2

static inline void disable_dcdc_pfm_mode(void)
{
	u8 val;

	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_OUTPUT_CTL1, &val);
	val |= (0x01 << 3); /*dcdc4 for gpu pwm mode*/
	val |= (0x01 << 4); /*dcdc5 for dram pwm mode*/
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_OUTPUT_CTL1, val);

	/* disable dcm mode for GPU stability Vdrop issue*/
	/* for old pmu */
	/*
	pmic_bus_write(AXP2202_RUNTIME_ADDR, 0xff, 0x0);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, 0xf4, 0x6);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, 0xf2, 0x4);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, 0xf5, 0x4);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, 0xff, 0x1);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, 0x12, 0x40);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, 0xff, 0x0);
	*/
}

int axp2202_probe_power_key(void)
{
	u8 reg_value;

	if (pmic_bus_read(AXP2202_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
			  &reg_value)) {
		return -1;
	}
	/* POKLIRQ,POKSIRQ */
	reg_value &= (0x03 << PMU_POWER_KEY_OFFSET);
	if (reg_value) {
		if (pmic_bus_write(AXP2202_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
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
		if (pmic_bus_read(AXP2202_RUNTIME_ADDR, p_item->cfg_reg_addr,
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

		if (pmic_bus_write(AXP2202_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP2202_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP2202_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}

int axp2202_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdc3", set_vol, 1);
}

int axp2202_set_efuse_voltage(int set_vol)
{
	return pmu_set_vol("cldo1", set_vol, 1);
}

int axp2202_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("dcdc1", set_vol, 1);
}

int axp2202_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdc2", set_vol, onoff);
}

int axp2202_set_sys_voltage_ext(char *name, int set_vol, int onoff)
{
	return pmu_set_vol(name, set_vol, onoff);
}


static int axp2202_set_necessary_reg(void)
{
	u8 reg_value;

	/* limit charge current to 300mA */
	reg_value = 0x9;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_CHARGE1, reg_value);

	/* limit run current to 2A */
	reg_value = 0x26;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_VBUS_CUR_SET, reg_value);

	/* set adc channel0 enable */
	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_ADC_CH0, &reg_value);
	reg_value |= 0x33;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_ADC_CH0, reg_value);


	/*set dcdc1 pwm mode*/
	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_OUTPUT_CTL1, &reg_value);
	reg_value |= (1 << 1);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_OUTPUT_CTL1, reg_value);

	/*pmu disable soften3 signal*/
/*	reg_value = 0x00;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_TWI_ADDR_EXT, reg_value);
	reg_value = 0x06;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_EFUS_OP_CFG, reg_value);
	reg_value = 0x04;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_EFREQ_CTRL, reg_value);
	reg_value = 0x01;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_TWI_ADDR_EXT, reg_value);
	reg_value = 0x30;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_SELLP_CFG, reg_value);
	reg_value = 0x00;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_TWI_ADDR_EXT, reg_value);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_EFREQ_CTRL, reg_value);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_EFUS_OP_CFG, reg_value);
*/
	/*pmu set vsys min*/
	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_VSYS_MIN, &reg_value);
	reg_value = 0x06;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_VSYS_MIN, reg_value);

	/*pmu set vimdpm cfg*/
	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_VBUS_VOL_SET, &reg_value);
	reg_value = 0x09;
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_VBUS_VOL_SET, reg_value);

	/*pmu reset enable*/
	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_OFF_CTL, &reg_value);
	reg_value |= (3 << 2);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_OFF_CTL, reg_value);

	/*pmu pwroff enable*/
/*	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_PWEON_PWEOFF_EN, &reg_value);
	reg_value |= (1 << 1);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_PWEON_PWEOFF_EN, reg_value);
*/
	/*pmu dcdc1 uvp disable */
	pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_DCDC_PWEOFF_EN, &reg_value);
	reg_value &= ~(1 << 0);
	pmic_bus_write(AXP2202_RUNTIME_ADDR, AXP2202_DCDC_PWEOFF_EN, reg_value);

	return 0;
}

int axp2202_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP2202_DEVICE_ADDR, AXP2202_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP2202_RUNTIME_ADDR, AXP2202_CHIP_ID_EXT, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	if (pmu_type == 0x02) {
		/* pmu type AXP21 */
		printf("PMU: AXP2202\n");
		axp2202_set_necessary_reg();
		return AXP2202_CHIP_ID;
	}

	printf("unknow PMU\n");
	return -1;
}
