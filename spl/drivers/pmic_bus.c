/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
* SPDX-License-Identifier:	GPL-2.0+
 *
 * Sunxi PMIC bus access helpers
 *
 * The axp152 & axp209 use an i2c bus, the axp221 uses the p2wi bus and the
 * axp223 uses the rsb bus, these functions abstract this.
 *
 */

#include <common.h>
#include <arch/rsb.h>
#ifdef CFG_SUNXI_TWI
#include <arch/i2c.h>
#endif
#include <arch/pmic_bus.h>


#define AXP152_I2C_ADDR			0x30

#define AXP209_I2C_ADDR			0x34

#define AXP221_CHIP_ADDR		0x68
#define AXP221_CTRL_ADDR		0x3e
#define AXP221_INIT_DATA		0x3e

#define AXP858_DEVICE_ADDR              0x745
/* AXP818 device and runtime addresses are same as AXP223 */
#define AXP223_DEVICE_ADDR		0x3a3
#define AXP223_RUNTIME_ADDR		0x2d


int pmic_bus_init(u32 device_addr, u32 runtime_addr)
{
	/* This cannot be 0 because it is used in SPL before BSS is ready */
	/*static int needs_init = 1;*/
	__maybe_unused int ret = 0;

	/*if (!needs_init)
		return 0;
	*/
#if defined CFG_AXP81X_POWER || defined CFG_AXP809_POWER\
	|| defined CFG_AXP858_POWER  || defined CFG_AXP2101_POWER\
	|| defined CFG_AXP152_POWER || defined CFG_AXP1530_POWER\
	|| defined CFG_AXP806_POWER || defined CFG_AXP2202_POWER
# ifdef CONFIG_MACH_SUN6I
	p2wi_init();
	ret = p2wi_change_to_p2wi_mode(AXP221_CHIP_ADDR, AXP221_CTRL_ADDR,
				       AXP221_INIT_DATA);
# elif defined CFG_SUNXI_TWI
	i2c_init_cpus(device_addr, runtime_addr);
# else
	ret = rsb_init();
	if (ret)
		return ret;
	ret = rsb_set_device_address(device_addr, runtime_addr);
# endif
	if (ret)
		return ret;
#endif

	/*needs_init = 0;*/
	return 0;
}

int pmic_bus_read(u32 runtime_addr, u8 reg, u8 *data)
{
#if defined CFG_AXP221_POWER || defined CFG_AXP809_POWER || defined CFG_AXP81X_POWER\
	|| defined CFG_AXP858_POWER || defined CFG_AXP2101_POWER || CFG_AXP152_POWER\
	|| defined CFG_AXP1530_POWER || defined CFG_AXP806_POWER || defined CFG_AXP2202_POWER
#if defined CFG_SUNXI_TWI
	return i2c_read(runtime_addr, reg, 1, data, 1);
#else
	return rsb_read(runtime_addr, reg, data);
#endif
#elif defined FPGA_PLATFORM
	return 0;
#else
#error "pmic_bus_read -Werror=return-type"
#endif
}

int pmic_bus_write(u32 runtime_addr, u8 reg, u8 data)
{
#if defined CFG_AXP221_POWER || defined CFG_AXP809_POWER || defined CFG_AXP81X_POWER\
	|| defined CFG_AXP858_POWER || defined CFG_AXP2101_POWER || CFG_AXP152_POWER\
	|| defined CFG_AXP1530_POWER || defined CFG_AXP806_POWER || defined CFG_AXP2202_POWER
# ifdef CONFIG_MACH_SUN6I
	return p2wi_write(reg, data);
# elif defined CONFIG_MACH_SUN8I_R40 || defined CFG_SUNXI_TWI
	return i2c_write(runtime_addr, reg, 1, &data, 1);
# else
	return rsb_write(runtime_addr, reg, data);
# endif
#elif defined FPGA_PLATFORM
	return 0;
#else
#error "pmic_bus_write -Werror=return-type"
#endif
}

int pmic_bus_setbits(u32 runtime_addr, u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(runtime_addr, reg, &val);
	if (ret)
		return ret;

	val |= bits;
	return pmic_bus_write(runtime_addr, reg, val);
}

int pmic_bus_clrbits(u32 runtime_addr, u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(runtime_addr, reg, &val);
	if (ret)
		return ret;

	val &= ~bits;
	return pmic_bus_write(runtime_addr, reg, val);
}
