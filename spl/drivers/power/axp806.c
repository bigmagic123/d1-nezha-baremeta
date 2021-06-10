/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp806_reg.h>

static int pmu_set_vol(char *name, int set_vol, int onoff);

static axp_contrl_info axp_ctrl_tbl[] = {
	{ "dcdca", 600, 1520, AXP806_DCAOUT_VOL, 0x7f, AXP806_OUTPUT_CTL1, 0, 0,
	{ {600, 1100, 10}, {1120, 1520, 20}, } },

	{ "dcdcb", 1000, 2550, AXP806_DCBOUT_VOL, 0x1f, AXP806_OUTPUT_CTL1, 1, 0,
	{ {1000, 2550, 50}, } },

	{ "dcdcc", 600, 1520, AXP806_DCCOUT_VOL, 0x7f, AXP806_OUTPUT_CTL1, 2, 0,
	{ {600, 1100, 10}, {1120, 1520, 20}, } },

	{ "dcdcd", 600, 3300, AXP806_DCDOUT_VOL, 0x3f, AXP806_OUTPUT_CTL1, 3, 0,
	{ {600, 1500, 20}, {1600, 3300, 100}, } },

	{ "dcdce", 1100, 3400, AXP806_DCEOUT_VOL, 0x1f, AXP806_OUTPUT_CTL1, 4, 0,
	{ {1100, 3400, 100}, } },

	{ "aldo1", 700, 3300, AXP806_ALDO1OUT_VOL, 0x1f, AXP806_OUTPUT_CTL1, 5, 0,
	{ {700, 3300, 100}, } },

	{ "aldo2", 700, 3400, AXP806_ALDO2OUT_VOL, 0x1f, AXP806_OUTPUT_CTL1, 6, 0,
	{ {700, 3400, 100}, } },

	{ "aldo3", 700, 3300, AXP806_ALDO3OUT_VOL, 0x1f, AXP806_OUTPUT_CTL1, 7, 0,
	{ {700, 3300, 100}, } },

	{ "bldo1", 700, 1900, AXP806_BLDO1OUT_VOL, 0x0f, AXP806_OUTPUT_CTL2, 0, 0,
	{ {700, 1900, 100}, } },

	{ "bldo2", 700, 1900, AXP806_BLDO2OUT_VOL, 0x0f, AXP806_OUTPUT_CTL2, 1, 0,
	{ {700, 1900, 100}, } },

	{ "bldo3", 700, 1900, AXP806_BLDO3OUT_VOL, 0x0f, AXP806_OUTPUT_CTL2, 2, 0,
	{ {700, 1900, 100}, } },

	{ "bldo4", 700, 1900, AXP806_BLDO4OUT_VOL, 0x0f, AXP806_OUTPUT_CTL2, 3, 0,
	{ {700, 1900, 100}, } },

	{ "cldo1", 700, 3300, AXP806_CLDO1OUT_VOL, 0x1f, AXP806_OUTPUT_CTL2, 4, 0,
	{ {700, 3300, 100}, } },

	{ "cldo2", 700, 4200, AXP806_CLDO2OUT_VOL, 0x1f, AXP806_OUTPUT_CTL2, 5, 0,
	{ {700, 3400, 100}, {3600, 4200, 200}, } },

	{ "cldo3", 700, 3300, AXP806_CLDO3OUT_VOL, 0x1f, AXP806_OUTPUT_CTL2, 6, 0,
	{ {700, 3300, 100}, } },

};


int axp806_probe_power_key(void)
{
	u8 reg_value;

	if (pmic_bus_read(AXP806_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
			  &reg_value)) {
		return -1;
	}
	/* POKLIRQ,POKSIRQ */
	reg_value &= (0x03 << PMU_POWER_KEY_OFFSET);
	if (reg_value) {
		if (pmic_bus_write(AXP806_RUNTIME_ADDR, PMU_POWER_KEY_STATUS,
				   reg_value)) {
			return -1;
		}
	}

	return (reg_value >> PMU_POWER_KEY_OFFSET) & 3;
}

int axp806_get_power_source(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP806_RUNTIME_ADDR, AXP806_STARUP_SRC, &reg_value)) {
		return -1;
	}

	if (reg_value & (1 << 0)) {
		return AXP_BOOT_SOURCE_VBUS_USB;
	} else if (reg_value & (1 << 2)) {
		return AXP_BOOT_SOURCE_BUTTON;
	} else if (reg_value & (1 << 3)) {
		return AXP_BOOT_SOURCE_IRQ_LOW;
	}
	return -1;
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
		if (pmic_bus_read(AXP806_RUNTIME_ADDR, p_item->cfg_reg_addr,
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

		if (pmic_bus_write(AXP806_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP806_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP806_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}
	return 0;

}

int axp806_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol("dcdcd", set_vol, 1);
}


int axp806_set_pll_voltage(int set_vol)
{
	return pmu_set_vol("dcdca", set_vol, 1);
}

int axp806_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol("dcdcc", set_vol, onoff);
}

int axp806_set_sys_voltage_ext(char *name, int set_vol, int onoff)
{
	return pmu_set_vol(name, set_vol, onoff);
}

static int axp806_necessary_reg_enable(void)
{
	u8 reg_val;
	if (pmic_bus_read(AXP806_RUNTIME_ADDR, AXP806_DCMOD_CTL1, &reg_val)) {
		return -1;
	}
	reg_val |= 0x7;
	if (pmic_bus_write(AXP806_RUNTIME_ADDR, AXP806_DCMOD_CTL1, reg_val)) {
		return -1;
	}

	if (pmic_bus_read(AXP806_RUNTIME_ADDR, AXP806_DIASBLE, &reg_val)) {
		return -1;
	}
	/* PWROK drive low restart function enable  */
	reg_val |= (0x1U << 4);
	if (pmic_bus_write(AXP806_RUNTIME_ADDR, AXP806_DIASBLE, reg_val)) {
		return -1;
	}

	if (pmic_bus_read(AXP806_RUNTIME_ADDR, AXP806_IRQ_SETTING, &reg_val)) {
		return -1;
	}
	/* irq function enable  */
	reg_val |= (0x1U << 7);
	if (pmic_bus_write(AXP806_RUNTIME_ADDR, AXP806_IRQ_SETTING, reg_val)) {
		return -1;
	}

	return 0;
}

int axp806_reg_read(u8 addr, u8 *val)
{
	return pmic_bus_read(AXP806_RUNTIME_ADDR, addr, val);
}

int axp806_reg_write(u8 addr, u8 val)
{
	return pmic_bus_write(AXP806_RUNTIME_ADDR, addr, val);
}


int axp806_axp_init(u8 power_mode)
{
	u8 pmu_type;

	if (pmic_bus_init(AXP806_DEVICE_ADDR, AXP806_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP806_RUNTIME_ADDR, AXP806_IC_TYPE, &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0xCF;
	if (pmu_type == AXP806_CHIP_ID) {
		/* pmu type AXP806 */
		printf("PMU: AXP806\n");
		axp806_necessary_reg_enable();
		return AXP806_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}

