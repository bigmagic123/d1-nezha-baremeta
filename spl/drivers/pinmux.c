/*
 * (C) Copyright 2007-2011
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <arch/gpio.h>

void sunxi_gpio_set_cfgbank(struct sunxi_gpio *pio, int bank_offset, u32 val)
{
	u32 index = GPIO_CFG_INDEX(bank_offset);
	u32 offset = GPIO_CFG_OFFSET(bank_offset);

	clrsetbits_le32(&pio->cfg[0] + index, 0xf << offset, val << offset);
}

void sunxi_gpio_set_cfgpin(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	sunxi_gpio_set_cfgbank(pio, pin, val);
}

int sunxi_gpio_get_cfgbank(struct sunxi_gpio *pio, int bank_offset)
{
	u32 index = GPIO_CFG_INDEX(bank_offset);
	u32 offset = GPIO_CFG_OFFSET(bank_offset);
	u32 cfg;

	cfg = readl(&pio->cfg[0] + index);
	cfg >>= offset;

	return cfg & 0xf;
}

int sunxi_gpio_get_cfgpin(u32 pin)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	return sunxi_gpio_get_cfgbank(pio, pin);
}

int sunxi_gpio_set_drv(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_DRV_INDEX(pin);
	u32 offset = GPIO_DRV_OFFSET(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	clrsetbits_le32(&pio->drv[0] + index, 0x3 << offset, val << offset);

	return 0;
}

int sunxi_gpio_set_pull(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_PULL_INDEX(pin);
	u32 offset = GPIO_PULL_OFFSET(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);
	clrsetbits_le32(&pio->pull[0] + index, 0x3 << offset, val << offset);

	return 0;
}


int boot_set_gpio(void  *user_gpio_list, u32 group_count_max, int set_gpio)
{
	normal_gpio_set_t *tmp_user_gpio_data, *gpio_list;
	u32 first_port;
	u32 tmp_group_func_data;
	u32 tmp_group_pull_data;
	u32 tmp_group_dlevel_data;
	u32 tmp_group_data_data;
	u32 data_change = 0;
	u32 port, port_num, port_num_func, port_num_pull;
	u32 pre_port, pre_port_num_func;
	u32 pre_port_num_pull;
	volatile u32 *tmp_group_func_addr, *tmp_group_pull_addr;
	volatile u32 *tmp_group_dlevel_addr, *tmp_group_data_addr;
	int i, tmp_val;

	gpio_list = (normal_gpio_set_t *)user_gpio_list;

	for (first_port = 0; first_port < group_count_max; first_port++) {
		tmp_user_gpio_data = gpio_list + first_port;
		port = tmp_user_gpio_data->port;
		port_num = tmp_user_gpio_data->port_num;
		if (!port) {
			continue;
		}
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);
		if (port < 12) {
			tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
			tmp_group_pull_addr = PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_pull);
			tmp_group_data_addr = PIO_REG_DATA(port);
		} else {
			tmp_group_func_addr = R_PIO_REG_CFG(port, port_num_func);
			tmp_group_pull_addr = R_PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr = R_PIO_REG_DLEVEL(port, port_num_pull);
			tmp_group_data_addr = R_PIO_REG_DATA(port);
		}

		tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
		tmp_group_pull_data = GPIO_REG_READ(tmp_group_pull_addr);
		tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
		tmp_group_data_data = GPIO_REG_READ(tmp_group_data_addr);

		pre_port = port;
		pre_port_num_func = port_num_func;
		pre_port_num_pull = port_num_pull;
		/* update funtion */
		tmp_val = (port_num - (port_num_func << 3)) << 2;
		tmp_group_func_data &= ~(PIO_CFG_MASK << tmp_val);
		if (set_gpio) {
			tmp_group_func_data |=
			    (tmp_user_gpio_data->mul_sel & PIO_CFG_MASK) << tmp_val;
		}
		/* update pull  */
		tmp_val = (port_num - (port_num_pull << 4)) << 1;
		if (tmp_user_gpio_data->pull >= 0) {
			tmp_group_pull_data &= ~(0x03 << tmp_val);
			tmp_group_pull_data |= (tmp_user_gpio_data->pull & 0x03)
					       << tmp_val;
		}
		/* update driver level */
		if (tmp_user_gpio_data->drv_level >= 0) {
			tmp_group_dlevel_data &= ~(0x03 << tmp_val);
			tmp_group_dlevel_data |=
			    (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
		}
		/* update data */
		if (tmp_user_gpio_data->mul_sel == 1) {
			if (tmp_user_gpio_data->data >= 0) {
				tmp_val = tmp_user_gpio_data->data & 1;
				tmp_group_data_data &= ~(1 << port_num);
				tmp_group_data_data |= tmp_val << port_num;
				data_change = 1;
			}
		}

		break;
	}
	
	if (first_port >= group_count_max) {
		return -1;
	}

	for (i = first_port + 1; i < group_count_max; i++) {
		tmp_user_gpio_data = gpio_list + i;
		port = tmp_user_gpio_data->port;
		port_num = tmp_user_gpio_data->port_num;
		if (!port) {
			break;
		}
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);

		if ((port_num_pull != pre_port_num_pull) ||
		    (port != pre_port)) {
			GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
			GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);
			GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
			if (data_change) {
				data_change = 0;
				GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
			}

			tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
			tmp_group_pull_addr = PIO_REG_PULL(port, port_num_pull);
			tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_pull);
			tmp_group_data_addr = PIO_REG_DATA(port);

			tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
			tmp_group_pull_data = GPIO_REG_READ(tmp_group_pull_addr);
			tmp_group_dlevel_data = GPIO_REG_READ(tmp_group_dlevel_addr);
			tmp_group_data_data = GPIO_REG_READ(tmp_group_data_addr);
		} else if (pre_port_num_func != port_num_func) {
			GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
			tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
			tmp_group_func_data = GPIO_REG_READ(tmp_group_func_addr);
		}

		pre_port_num_pull = port_num_pull;
		pre_port_num_func = port_num_func;
		pre_port = port;

		tmp_val = (port_num - (port_num_func << 3)) << 2;
		if (tmp_user_gpio_data->mul_sel >= 0) {
			tmp_group_func_data &= ~(PIO_CFG_MASK << tmp_val);
			if (set_gpio) {
				tmp_group_func_data |=
				    (tmp_user_gpio_data->mul_sel & PIO_CFG_MASK)
				    << tmp_val;
			}
		}

		tmp_val = (port_num - (port_num_pull << 4)) << 1;
		if (tmp_user_gpio_data->pull >= 0) {
			tmp_group_pull_data &= ~(0x03 << tmp_val);
			tmp_group_pull_data |= (tmp_user_gpio_data->pull & 0x03)
					       << tmp_val;
		}

		if (tmp_user_gpio_data->drv_level >= 0) {
			tmp_group_dlevel_data &= ~(0x03 << tmp_val);
			tmp_group_dlevel_data |=
			    (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
		}

		if (tmp_user_gpio_data->mul_sel == 1) {
			if (tmp_user_gpio_data->data >= 0) {
				tmp_val = tmp_user_gpio_data->data & 1;
				tmp_group_data_data &= ~(1 << port_num);
				tmp_group_data_data |= tmp_val << port_num;
				data_change = 1;
			}
		}
	}

	if (tmp_group_func_addr) {
		GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);
		GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);
		GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data);
		if (data_change) {
			GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data);
		}
	}

	return 0;
}
