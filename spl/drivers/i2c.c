/*
 * (C) Copyright 2016
* SPDX-License-Identifier:	GPL-2.0+
 *Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *weidonghui <weidonghui@allwinnertech.com>
 *
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <arch/i2c.h>
#include <arch/gpio.h>
#include <arch/clock.h>
#include <private_boot0.h>

#define I2C_WRITE 0
#define I2C_READ 1

#define I2C_OK 0
#define I2C_NOK 1
#define I2C_NACK 2
#define I2C_NOK_LA 3 /* Lost arbitration */
#define I2C_NOK_TOUT 4 /* time out */

#define I2C_START_TRANSMIT 0x08
#define I2C_RESTART_TRANSMIT 0x10
#define I2C_ADDRWRITE_ACK 0x18
#define I2C_ADDRREAD_ACK 0x40
#define I2C_DATAWRITE_ACK 0x28
#define I2C_READY 0xf8
#define I2C_DATAREAD_NACK 0x58
#define I2C_DATAREAD_ACK 0x50

normal_gpio_cfg *twi_gpio;
int i2c_has_init;

#if defined(CONFIG_ARCH_SUN50IW10)
normal_gpio_cfg i2c_gpio[4] = {
    {12, 0, 2, 1, 0, 0, {SUNXI_PHY_R_I2C0} }, /* PL0: 2--SCK */
    {12, 1, 2, 1, 0, 0, {SUNXI_PHY_R_I2C0} }, /* PL1: 2--SDA */
};
#else
normal_gpio_cfg i2c_gpio[4] = {
    {12, 0, 3, 1, 0, 0, {SUNXI_PHY_R_I2C0} }, /* pl0: 3--sck */
    {12, 1, 3, 1, 0, 0, {SUNXI_PHY_R_I2C0} }, /* pl1: 3--sda */
};
#endif

/* status or interrupt source */
/*------------------------------------------------------------------------------
 * Code   Status
 * 00h	  Bus error
 * 08h	  START condition transmitted
 * 10h	  Repeated START condition transmitted
 * 18h	  Address + Write bit transmitted, ACK received
 * 20h	  Address + Write bit transmitted, ACK not received
 * 28h	  Data byte transmitted in master mode, ACK received
 * 30h	  Data byte transmitted in master mode, ACK not received
 * 38h	  Arbitration lost in address or data byte
 * 40h	  Address + Read bit transmitted, ACK received
 * 48h	  Address + Read bit transmitted, ACK not received
 * 50h	  Data byte received in master mode, ACK transmitted
 * 58h	  Data byte received in master mode, not ACK transmitted
 * 60h	  Slave address + Write bit received, ACK transmitted
 * 68h	  Arbitration lost in address as master, slave address + Write bit received, ACK transmitted
 * 70h	  General Call address received, ACK transmitted
 * 78h	  Arbitration lost in address as master, General Call address received, ACK transmitted
 * 80h	  Data byte received after slave address received, ACK transmitted
 * 88h	  Data byte received after slave address received, not ACK transmitted
 * 90h	  Data byte received after General Call received, ACK transmitted
 * 98h	  Data byte received after General Call received, not ACK transmitted
 * A0h	  STOP or repeated START condition received in slave mode
 * A8h	  Slave address + Read bit received, ACK transmitted
 * B0h	  Arbitration lost in address as master, slave address + Read bit received, ACK transmitted
 * B8h	  Data byte transmitted in slave mode, ACK received
 * C0h	  Data byte transmitted in slave mode, ACK not received
 * C8h	  Last byte transmitted in slave mode, ACK received
 * D0h	  Second Address byte + Write bit transmitted, ACK received
 * D8h	  Second Address byte + Write bit transmitted, ACK not received
 * F8h	  No relevant status information or no interrupt
 *-----------------------------------------------------------------------------*/

static struct sunxi_twi_reg *i2c;



static void i2c_debug(void)
{
	i2c_info("i2c->addr  :\t0x%x:0x%x\n", &i2c->addr, i2c->addr);
	i2c_info("i2c->xaddr :\t0x%x:0x%x\n", &i2c->xaddr, i2c->xaddr);
	i2c_info("i2c->data  :\t0x%x:0x%x\n", &i2c->data, i2c->data);
	i2c_info("i2c->ctl   :\t0x%x:0x%x\n", &i2c->ctl, i2c->ctl);
	i2c_info("i2c->status:\t0x%x:0x%x\n", &i2c->status, i2c->status);
	i2c_info("i2c->clk   :\t0x%x:0x%x\n", &i2c->clk, i2c->clk);
	i2c_info("i2c->srst  :\t0x%x:0x%x\n", &i2c->srst, i2c->srst);
	i2c_info("i2c->eft   :\t0x%x:0x%x\n", &i2c->eft, i2c->eft);
	i2c_info("i2c->lcr   :\t0x%x:0x%x\n", &i2c->lcr, i2c->lcr);
	i2c_info("i2c->dvfs  :\t0x%x:0x%x\n", &i2c->dvfs, i2c->dvfs);

}
static __s32 i2c_sendbyteaddr(__u32 byteaddr)
{
	__s32 time = 0xffff;
	__u32 tmp_val;

	i2c->data = byteaddr & 0xff;
	i2c->ctl |= (0x01 << 3); /*write 1 to clean int flag*/

	while ((time--) && (!(i2c->ctl & 0x08)))
		;
	if (time <= 0) {
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
	i2c_debug();
	if (tmp_val != I2C_DATAWRITE_ACK) {
		return -I2C_DATAWRITE_ACK;
	}

	return I2C_OK;
}

static __s32 i2c_sendstart(void)
{
	__s32 time = 0xffff;
	__u32 tmp_val;

	i2c->eft  = 0;
	i2c->srst = 1;
	i2c->ctl  |= TWI_CTL_STA;

	while ((time--) && (!(i2c->ctl & TWI_CTL_INTFLG)))
		;
	if (time <= 0) {
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
	if (tmp_val != I2C_START_TRANSMIT) {
		return -I2C_START_TRANSMIT;
	}

	return I2C_OK;
}


static __s32 i2c_sendslaveaddr(__u32 saddr, __u32 rw)
{
	__s32 time = 0xffff;
	__u32 tmp_val;

	rw &= 1;
	i2c->data = ((saddr & 0xff) << 1) | rw;
	i2c->ctl |= TWI_CTL_INTFLG; /*write 1 to clean int flag*/
	while ((time--) && (!(i2c->ctl & TWI_CTL_INTFLG)))
		;
	if (time <= 0) {
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
	if (rw == I2C_WRITE) /*+write*/
	{
		if (tmp_val != I2C_ADDRWRITE_ACK) {
			return -I2C_ADDRWRITE_ACK;
		}
	}

	else /*+read*/
	{
		if (tmp_val != I2C_ADDRREAD_ACK) {
			return -I2C_ADDRREAD_ACK;
		}
	}
	i2c_debug();

	return I2C_OK;
}

static __s32 i2c_sendRestart(void)
{
	__s32 time = 0xffff;
	__u32 tmp_val;
	tmp_val = i2c->ctl;

	tmp_val |= 0x20;
	i2c->ctl = tmp_val;

	while ((time--) && (!(i2c->ctl & 0x08)))
		;
	if (time <= 0) {
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
	if (tmp_val != I2C_RESTART_TRANSMIT) {
		return -I2C_RESTART_TRANSMIT;
	}

	return I2C_OK;
}

static __s32 i2c_stop(void)
{
	__s32 time = 0xffff;
	__u32 tmp_val;
	i2c->ctl |= (0x01 << 4);
	i2c->ctl |= (0x01 << 3);
	while ((time--) && (i2c->ctl & 0x10))
		;
	if (time <= 0) {
		return -I2C_NOK_TOUT;
	}
	time = 0xffff;
	while ((time--) && (i2c->status != I2C_READY))
		;
	tmp_val = i2c->status;
	if (tmp_val != I2C_READY) {
		return -I2C_NOK_TOUT;
	}

	return I2C_OK;
}

static __s32 i2c_getdata(__u8 *data_addr, __u32 data_count)
{
	__s32 time = 0xffff;
	__u32 tmp_val;
	__u32 i;
	if (data_count == 1) {
		i2c->ctl |= (0x01 << 3);
		while ((time--) && (!(i2c->ctl & 0x08)))
			;
		if (time <= 0) {
			return -I2C_NOK_TOUT;
		}
		for (time = 0; time < 100; time++)
			;
		*data_addr = i2c->data;

		tmp_val = i2c->status;
		if (tmp_val != I2C_DATAREAD_NACK) {
			return -I2C_DATAREAD_NACK;
		}
	} else {
		for (i = 0; i < data_count - 1; i++) {
			time = 0xffff;
			/*host should send ack every time when a data packet finished*/
			tmp_val = i2c->ctl | (0x01 << 2);
			tmp_val = i2c->ctl | (0x01 << 3);
			tmp_val |= 0x04;
			i2c->ctl = tmp_val;
			/*i2c->ctl |=(0x01<<3);*/

			while ((time--) && (!(i2c->ctl & 0x08)))
				;
			if (time <= 0) {
				return -I2C_NOK_TOUT;
			}
			for (time = 0; time < 100; time++)
				;
			time	 = 0xffff;
			data_addr[i] = i2c->data;
			while ((time--) && (i2c->status != I2C_DATAREAD_ACK))
				;
			if (time <= 0) {
				return -I2C_NOK_TOUT;
			}
		}

		time = 0xffff;
		i2c->ctl &= 0xFb; /*the last data packet,not send ack*/
		i2c->ctl |= (0x01 << 3);
		while ((time--) && (!(i2c->ctl & 0x08)))
			;
		if (time <= 0) {
			return -I2C_NOK_TOUT;
		}
		for (time = 0; time < 100; time++)
			;
		data_addr[data_count - 1] = i2c->data;
		while ((time--) && (i2c->status != I2C_DATAREAD_NACK))
			;
		if (time <= 0) {
			return -I2C_NOK_TOUT;
		}
	}

	return I2C_OK;
}

int i2c_read(u8 chip, uint addr, int alen, u8 *buffer, int len)
{
	int i, ret, addrlen;
	char *slave_reg;
	if (!i2c_has_init) {
		return -I2C_NOK_TOUT;
	}
	ret  = i2c_sendstart();
	if (ret) {
		goto i2c_read_err_occur;
	}

	ret = i2c_sendslaveaddr(chip, I2C_WRITE);
	if (ret) {
		goto i2c_read_err_occur;
	}
	/*send byte address*/

	if (alen >= 3) {
		addrlen = 2;
	} else if (alen <= 1) {
		addrlen = 0;
	} else {
		addrlen = 1;
	}
	slave_reg = (char *)&addr;

	for (i = addrlen; i >= 0; i--) {
		ret = i2c_sendbyteaddr(slave_reg[i] & 0xff);
		if (ret) {
			goto i2c_read_err_occur;
		}
	}

	ret = i2c_sendRestart();
	if (ret) {
		goto i2c_read_err_occur;
	}

	ret = i2c_sendslaveaddr(chip, I2C_READ);
	if (ret) {
		goto i2c_read_err_occur;
	}
	/*get data*/

	ret = i2c_getdata(buffer, len);
	if (ret) {
		goto i2c_read_err_occur;
	}

i2c_read_err_occur:
	i2c_stop();

	return ret;
}

static __s32 i2c_senddata(__u8 *data_addr, __u32 data_count)
{
	__s32 time = 0xffff;
	__u32 i;

	for (i = 0; i < data_count; i++) {
		time      = 0xffff;
		i2c->data = data_addr[i];
#if defined(CONFIG_ARCH_SUN5I) || defined(CONFIG_ARCH_SUN7I)
		i2c->ctl &= 0xF7;
#else
		i2c->ctl |= (0x01 << 3);
#endif
		while ((time--) && (!(i2c->ctl & 0x08)))
			;
		if (time <= 0) {
			return -I2C_NOK_TOUT;
		}
		time = 0xffff;
		while ((time--) && (i2c->status != I2C_DATAWRITE_ACK)) {
			;
		}
		if (time <= 0) {
			return -I2C_NOK_TOUT;
		}
	}

	return I2C_OK;
}

int i2c_write(u8 chip, uint addr, int alen, u8 *buffer, int len)
{
	int i, ret, ret0, addrlen;
	char *slave_reg;

	if (!i2c_has_init) {
		return -I2C_NOK_TOUT;
	}
	ret0 = -1;
	ret  = i2c_sendstart();
	if (ret) {
		goto i2c_write_err_occur;
	}

	ret = i2c_sendslaveaddr(chip, I2C_WRITE);
	if (ret) {
		goto i2c_write_err_occur;
	}
	/*send byte address*/

	if (alen >= 3) {
		addrlen = 2;
	} else if (alen <= 1) {
		addrlen = 0;
	} else {
		addrlen = 1;
	}
	slave_reg = (char *)&addr;
	for (i = addrlen; i >= 0; i--) {
		ret = i2c_sendbyteaddr(slave_reg[i] & 0xff);
		if (ret) {
			goto i2c_write_err_occur;
		}
	}

	ret = i2c_senddata(buffer, len);
	if (ret) {
		goto i2c_write_err_occur;
	}
	ret0 = 0;

i2c_write_err_occur:
	i2c_stop();

	return ret0;
}

int axp_i2c_write(unsigned char chip, unsigned char addr, unsigned char data)
{
	return i2c_write(chip, addr, 1, &data, 1);
}

int axp_i2c_read(unsigned char chip, unsigned char addr, unsigned char *buffer)
{
	return i2c_read(chip, addr, 1, buffer, 1);
}

void i2c_set_clock(int speed, int slaveaddr)
{
	int i, clk_n, clk_m, pow_2_clk_n;
	/* reset i2c control  */
	i	 = 0xffff;
	i2c->srst = 1;
	while ((i2c->srst) && (i)) {
		i--;
	}
	if ((i2c->lcr & 0x30) != 0x30) {
		/* toggle I2C SCL and SDA until bus idle */
		i2c->lcr = 0x05;
		udelay(500);
		i = 10;
		while ((i > 0) && ((i2c->lcr & 0x02) != 2)) {
			/*control scl and sda output high level*/
			i2c->lcr |= 0x08;
			i2c->lcr |= 0x02;
			udelay(1000);
			/*control scl and sda output low level*/
			i2c->lcr &= ~0x08;
			i2c->lcr &= ~0x02;
			udelay(1000);
			i--;
		}
		i2c->lcr = 0x0;
		udelay(500);
	}
	speed /= 1000; /*khz*/

	if (speed < 100)
		speed = 100;
	else if (speed > 400)
		speed = 400;
	/*Foscl=24000/(2^CLK_N*(CLK_M+1)*10)*/
	clk_n = (speed == 100) ? 1 : 0;
	pow_2_clk_n = 1;
	for (i = 0; i < clk_n; ++i)
		pow_2_clk_n *= 2;
	clk_m = (24000 / 10) / (pow_2_clk_n * speed) - 1;


	i2c->clk = (clk_m << 3) | clk_n;
	i2c->ctl |= 0x40;
	i2c->eft = 0;
	i2c_debug();
}

static void sunxi_i2c_bus_setting(u32 i2c_base, int onoff)
{
	int reg_value = 0;

	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	u32 bus_num = (i2c_base - SUNXI_TWI0_BASE)/0x400;
	if (bus_num <= 5) {
#if defined(CONFIG_SUNXI_VERSION1)
		if (onoff) {
			//de-assert
			reg_value = readl(&ccm->apb2_reset_cfg);
			reg_value |= (1 << bus_num);
			writel(reg_value, &ccm->apb2_reset_cfg);

			//gating clock pass
			reg_value = readl(&ccm->apb2_gate);
			reg_value &= ~(1 << bus_num);
			writel(reg_value, &ccm->apb2_gate);
			mdelay(1);
			reg_value |= (1 << bus_num);
			writel(reg_value, &ccm->apb2_gate);
		} else {
			//gating clock mask
			reg_value = readl(&ccm->apb2_gate);
			reg_value &= ~(1 << bus_num);
			writel(reg_value, &ccm->apb2_gate);

			//assert
			reg_value = readl(&ccm->apb2_reset_cfg);
			reg_value &= ~(1 << bus_num);
			writel(reg_value, &ccm->apb2_reset_cfg);
		}
#else
		if (onoff) {
			//de-assert
			reg_value = readl(&ccm->twi_gate_reset);
			reg_value |= (1 << (16 + bus_num));
			writel(reg_value, &ccm->twi_gate_reset);

			//gating clock pass
			reg_value = readl(&ccm->twi_gate_reset);
			reg_value &= ~(1 << bus_num);
			writel(reg_value, &ccm->twi_gate_reset);
			mdelay(1);
			reg_value |= (1 << bus_num);
			writel(reg_value, &ccm->twi_gate_reset);
		} else {
			//gating clock mask
			reg_value = readl(&ccm->twi_gate_reset);
			reg_value &= ~(1 << bus_num);
			writel(reg_value, &ccm->twi_gate_reset);

			//assert
			reg_value = readl(&ccm->twi_gate_reset);
			reg_value &= ~(1 << (16 + bus_num));
			writel(reg_value, &ccm->twi_gate_reset);
		}
#endif
	} else {
		int r_bus_num = (i2c_base - SUNXI_RTWI_BASE)/0x400;
		if (onoff) {
			/*de-assert*/
			reg_value = readl(SUNXI_RTWI_BRG_REG);
			reg_value &= ~(1 << (16 + r_bus_num));
			writel(reg_value, SUNXI_RTWI_BRG_REG);

			reg_value = readl(SUNXI_RTWI_BRG_REG);
			reg_value |= (1 << (16 + r_bus_num));
			writel(reg_value, SUNXI_RTWI_BRG_REG);

			/*gating clock pass*/
			reg_value = readl(SUNXI_RTWI_BRG_REG);
			reg_value &= ~(1 << r_bus_num);
			writel(reg_value, SUNXI_RTWI_BRG_REG);
			mdelay(1);
			reg_value |= (1 << r_bus_num);
			writel(reg_value, SUNXI_RTWI_BRG_REG);
		} else {
			/*gating clock mask*/
			reg_value = readl(SUNXI_RTWI_BRG_REG);
			reg_value &= ~(1 << r_bus_num);
			writel(reg_value, SUNXI_RTWI_BRG_REG);

			/*assert*/
			reg_value = readl(SUNXI_RTWI_BRG_REG);
			reg_value &= ~(1 << (16 + r_bus_num));
			writel(reg_value, SUNXI_RTWI_BRG_REG);
		}
	}
}

normal_gpio_cfg *get_i2c_gpio(void)
{
	normal_gpio_cfg *boot_i2c_gpio, *temp_gpio;
#ifdef CFG_SUNXI_SBOOT
	boot_i2c_gpio = (normal_gpio_cfg *)sboot_head.i2c_gpio;
#elif CFG_SUNXI_FES
	boot_i2c_gpio = (normal_gpio_cfg *)fes1_head.i2c_gpio;
#else
	boot_i2c_gpio = (normal_gpio_cfg *)BT0_head.i2c_gpio;
#endif
	if (boot_i2c_gpio->port == 0) {
	/*
	 * Setting a non-existent i2c pin
	 * will cause a very long timeout waiting
	 * so enable CFG_SUNXI_AUTO_TWI
	 */
#ifdef CFG_SUNXI_AUTO_TWI
		temp_gpio = NULL;
#else
		temp_gpio = i2c_gpio;
#endif
	} else {
		temp_gpio = boot_i2c_gpio;
	}
	return temp_gpio;
}

void i2c_init(ulong i2c_base, int speed, int slaveaddr)
{
	i2c = (struct sunxi_twi_reg *)i2c_base;
	boot_set_gpio(twi_gpio, 2, 1);
	sunxi_i2c_bus_setting(i2c_base, 1);
	i2c_set_clock(speed, slaveaddr);
}

void i2c_init_cpus(int speed, int slaveaddr)
{
	if (!i2c_has_init) {
		twi_gpio = get_i2c_gpio();
		if (twi_gpio != NULL) {
			i2c_init(twi_gpio[0].reserved[0] != SUNXI_PHY_R_I2C0 ?
				(SUNXI_TWI0_BASE + 0x400 * twi_gpio[0].reserved[0]) :
				(SUNXI_RTWI_BASE + 0x400 * (twi_gpio[0].reserved[0] - SUNXI_PHY_R_I2C0)),
				speed, slaveaddr);
			i2c_has_init = 1;
		} else {
			i2c_has_init = 0;
		}
	}
	return;
}

void i2c_exit(void)
{
	int i;
	u32 i2c_base;
	if (i2c_has_init) {
		twi_gpio = get_i2c_gpio();
		i2c_base = (twi_gpio[0].reserved[0] != SUNXI_PHY_R_I2C0 ?
				(SUNXI_TWI0_BASE + 0x400 * twi_gpio[0].reserved[0]) :
				(SUNXI_RTWI_BASE + 0x400 * (twi_gpio[0].reserved[0] - SUNXI_PHY_R_I2C0)));
		sunxi_i2c_bus_setting(i2c_base, 0);
		for (i = 0; i < 2; i++) {
			twi_gpio[i].mul_sel = 0x0;
			twi_gpio[i].pull = 0;
			twi_gpio[i].drv_level = 0x0;
		}
		boot_set_gpio(twi_gpio, 2, 1);
		i2c_has_init = 0;
	}
}
