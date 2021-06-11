// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2011-2020 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include "standby_twi.h"
#include "clk.h"
#include "../pin/standby_pin.h"
#include "../clk/standby_clk.h"

#ifdef CFG_TWI_USED

#define TWI_CHECK_TIMEOUT       (0x4ffff)

static __twic_reg_t *TWI_REG_BASE[3] = {
	(__twic_reg_t *) (TWI0_BASE),
	(__twic_reg_t *) (TWI1_BASE),
	(__twic_reg_t *) (TWI2_BASE)
};

static u32 TwiClkRegBak;
static u32 TwiCtlRegBak;
static __twic_reg_t *twi_reg = (__twic_reg_t *) (0);

/*
***********************************************************************
*                                   standby_twi_init
*
*Description: init twi transfer when apb2 clk == 24M.
*
*Arguments  :
*
*Return     :
*
***********************************************************************
*/
s32 standby_twi_init(sram_head_t *para)
{
	u32 group = para->pmu_port;
#ifdef CFG_SRAM_DT
	u32 clk_id = (group < TWI_NUM) ? (CCU_MOD_CLK_TWI0 + group)
		: (CCU_MOD_CLK_R_TWI0 + group - TWI_NUM);

	/* twi pin init */
	gpio_set_multi_sel(para->twi_para.sda.pin_grp,
			para->twi_para.sda.pin_num,
			para->twi_para.sda.mode);
	gpio_set_pull(para->twi_para.sda.pin_grp,
			para->twi_para.sda.pin_num,
			para->twi_para.sda.pupd);
	gpio_set_drive(para->twi_para.sda.pin_grp,
			para->twi_para.sda.pin_num,
			para->twi_para.sda.drv);

	gpio_set_multi_sel(para->twi_para.sck.pin_grp,
			para->twi_para.sck.pin_num,
			para->twi_para.sck.mode);
	gpio_set_pull(para->twi_para.sck.pin_grp,
			para->twi_para.sck.pin_num,
			para->twi_para.sck.pupd);
	gpio_set_drive(para->twi_para.sck.pin_grp,
			para->twi_para.sck.pin_num,
			para->twi_para.sck.drv);

	/* clock init */
	ccu_set_mclk_onoff(clk_id, CCU_CLK_ON);
	ccu_reset_module(clk_id);

	/* reg base init */
	twi_reg = (__twic_reg_t *)para->twi_para.reg_base;

	twi_reg->reg_ctl = (1 << 6) | (1 << 2);
#else
	twi_reg = TWI_REG_BASE[group];
#endif

	TwiClkRegBak = twi_reg->reg_clkr;
	/* backup INT_EN; no need for BUS_EN(0xc0)  */
	TwiCtlRegBak = 0x80 & twi_reg->reg_ctl;
	/*twi_reg->reg_clkr = (2<<3)|3; //100k */
	/* M = 5, N = 0 : -> 24M/(2^N*(M+1)*10) = 24M/60=400k */
	twi_reg->reg_clkr = (5 << 3) | 0;

	twi_reg->reg_reset |= 0x1;
	while (twi_reg->reg_reset & 0x1)
		;

	return 0;
}


/*
***********************************************************************
*                                   standby_twi_init_losc
*
*Description: init twi transfer when apb2 clk == 32K.
*
*Arguments  :
*
*Return     :
*
***********************************************************************
*/
s32 standby_twi_init_losc(int group)
{
	/* M = 0, N = 0 : -> 24M/(2^N*(M+1)*10) = 32k/10=3.2k */
	twi_reg->reg_clkr = (5 << 3) | 0;
}

/*
*********************************************************************************************************
*                                   standby_twi_exit
*
*Description: exit twi transfer.
*
*Arguments  :
*
*Return     :
*
*********************************************************************************************************
*/
s32 standby_twi_exit(void)
{
	/* softreset twi module  */
	twi_reg->reg_reset |= 0x1;
	/* delay */
	/*change_runtime_env(); */
	/*delay_ms(10); */

	/* restore clock division */
	twi_reg->reg_clkr = TwiClkRegBak;
	/* restore INT_EN */
	twi_reg->reg_ctl |= TwiCtlRegBak;
	return 0;
}

/*
*********************************************************************************************************
*                                   _standby_twi_stop
*
*Description: stop current twi transfer.
*
*Arguments  :
*
*Return     :
*
*********************************************************************************************************
*/
static int _standby_twi_stop(void)
{
	unsigned int nop_read;
	unsigned int timeout = TWI_CHECK_TIMEOUT;

	twi_reg->reg_ctl = (twi_reg->reg_ctl & 0xc0) | 0x10 | 0x08;	/* set stop+clear int flag */

	nop_read = twi_reg->reg_ctl;
	nop_read = nop_read;
	/* 1. stop bit is zero. */
	while ((twi_reg->reg_ctl & 0x10) && (--timeout))
		;
	if (timeout == 0) {
		return -1;
	}
	/* 2. twi fsm is idle(0xf8). */
	timeout = TWI_CHECK_TIMEOUT;
	while ((0xf8 != twi_reg->reg_status) && (--timeout))
		;
	if (timeout == 0) {
		return -1;
	}
	/* 3. twi scl & sda must high level. */
	timeout = TWI_CHECK_TIMEOUT;
	while ((0x3a != twi_reg->reg_lctl) && (--timeout))
		;
	if (timeout == 0) {
		return -1;
	}

	return 0;
}

/*
*********************************************************************************************************
*                                   twi_byte_rw
*
*Description: twi byte read and write.
*
*Arguments  : op        operation read or write;
*             saddr     slave address;
*             baddr     byte address;
*             data      pointer to the data to be read or write;
*
*Return     : result;
*               = EPDK_OK,      byte read or write successed;
*               = EPDK_FAIL,    btye read or write failed!
*********************************************************************************************************
*/
s32 twi_byte_rw(enum twi_op_type_e op, u8 saddr, u8 baddr, u8 *data)
{
	unsigned char state_tmp;
	unsigned int timeout;
	int ret = -1;
	unsigned int ctrl;

	_standby_twi_stop();

	twi_reg->reg_efr = 0;	/* ��׼��д������0 */

	state_tmp = twi_reg->reg_status;
	if (state_tmp != 0xf8) {
		ret = 0xf8;
		goto stop_out;
	}

	/* control registser bitmap
	   7      6       5     4       3       2    1    0
	   INT_EN  BUS_EN  START  STOP  INT_FLAG  ACK  NOT  NOT
	 */

	/*1.Send Start */
	twi_reg->reg_ctl |= 0x20;
	timeout = TWI_CHECK_TIMEOUT;
	while ((!(twi_reg->reg_ctl & 0x08)) && (--timeout))
		;
	if (timeout == 0) {
		ret = 0xff;
		goto stop_out;
	}
	state_tmp = twi_reg->reg_status;
	if (state_tmp != 0x08) {
		ret = 0x08;
		goto stop_out;
	}

	/*2.Send Slave Address */
	twi_reg->reg_data = ((saddr << 1) & 0xfe) | 0;	/* slave address + write */
	twi_reg->reg_ctl &= 0xCF;
	timeout = TWI_CHECK_TIMEOUT;
	while ((!(twi_reg->reg_ctl & 0x08)) && (--timeout))
		;
	if (timeout == 0) {
		ret = 0xfe;
		goto stop_out;
	}
	state_tmp = twi_reg->reg_status;
	if (state_tmp != 0x18) {
		ret = 0x18;
		goto stop_out;
	}

	/*3.Send Byte Address */
	twi_reg->reg_data = baddr;
	twi_reg->reg_ctl &= 0xCF;
	timeout = TWI_CHECK_TIMEOUT;
	while ((!(twi_reg->reg_ctl & 0x08)) && (--timeout))
		;
	if (timeout == 0) {
		ret = 0xfd;
		goto stop_out;
	}
	state_tmp = twi_reg->reg_status;
	if (state_tmp != 0x28) {
		ret = 0x28;
		goto stop_out;
	}

	if (op == TWI_OP_WR) {
		/*4.Send Data to be write */
		twi_reg->reg_data = *data;
		twi_reg->reg_ctl &= 0xCF;
		timeout = TWI_CHECK_TIMEOUT;
		while ((!(twi_reg->reg_ctl & 0x08)) && (--timeout))
		;
		if (timeout == 0) {
			ret = 0xfc;
			goto stop_out;
		}
		state_tmp = twi_reg->reg_status;
		if (state_tmp != 0x28) {
			ret = 0x28;
			goto stop_out;
		}
	} else {
		/*4. Send restart for read */
		twi_reg->reg_ctl = (twi_reg->reg_ctl & 0xc0) | 0x20 | 0x08;	/* set start+clear int flag */
		timeout = TWI_CHECK_TIMEOUT;
		while ((!(twi_reg->reg_ctl & 0x08)) && (--timeout))
		;
		if (timeout == 0) {
			ret = 0xfb;
			goto stop_out;
		}
		state_tmp = twi_reg->reg_status;
		if (state_tmp != 0x10) {
			ret = 0x10;
			goto stop_out;
		}

		/*5.Send Slave Address */
		twi_reg->reg_data = (saddr << 1) | 1;	/* slave address+ read */
		twi_reg->reg_ctl &= 0xCF;	/* clear int flag then 0x40 come in */
		timeout = TWI_CHECK_TIMEOUT;
		while ((!(twi_reg->reg_ctl & 0x08)) && (--timeout))
		;
		if (timeout == 0) {
			ret = 0xfa;
			goto stop_out;
		}
		state_tmp = twi_reg->reg_status;
		if (state_tmp != 0x40) {
			ret = 0x40;
			goto stop_out;
		}

		/*6.Get data */
		twi_reg->reg_ctl &= 0xCF;	/* clear int flag then data come in */
		timeout = TWI_CHECK_TIMEOUT;
		while ((!(twi_reg->reg_ctl & 0x08)) && (--timeout))
		;
		if (timeout == 0) {
			ret = 0xf9;
			goto stop_out;
		}
		*data = twi_reg->reg_data;
		state_tmp = twi_reg->reg_status;
		if (state_tmp != 0x58) {
			ret = 0x58;
			goto stop_out;
		}
	}

	ret = 0;

      stop_out:
	/*WRITE: step 5; READ: step 7 */
	/*Send Stop */
	_standby_twi_stop();

	return ret;
}

#endif /* CFG_TWI_USED */
