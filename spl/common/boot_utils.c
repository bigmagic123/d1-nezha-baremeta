/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <spare_head.h>
#include <nand_boot0.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <arch/uart.h>

#ifdef CFG_SUNXI_USE_NEON
/*stdint.h included by arm_neon.h has conflict on uintptr_t with linux/types.h, undef it here*/
#undef __UINTPTR_TYPE__
#include <arm_neon.h>

int verify_addsum(void *mem_base, u32 size)
{
	uint32x4_t sum_vec;
	uint32x4_t adding;
	u32 neon_sum = 0;
	u32 src_sum;
	u32 *buf  = (u32 *)mem_base;
	u32 count = size >> 2;
	sbrom_toc1_head_info_t *bfh;
	bfh = (sbrom_toc1_head_info_t *)mem_base;

	neon_enable();

	src_sum	     = bfh->add_sum;
	bfh->add_sum = STAMP_VALUE;

	sum_vec = vdupq_n_u32(0);
	while (count >= 4) {
		adding	= vld1q_u32(buf);
		sum_vec = vaddq_u32(sum_vec, adding);
		buf += 4;
		count -= 4;
	}
	neon_sum += vgetq_lane_u32(sum_vec, 0);
	neon_sum += vgetq_lane_u32(sum_vec, 1);
	neon_sum += vgetq_lane_u32(sum_vec, 2);
	neon_sum += vgetq_lane_u32(sum_vec, 3);

	while (count-- > 0)
		neon_sum += *buf++;

		//printf("sum=%x\n", neon_sum);
		//printf("src_sum=%x\n", src_sum);

	bfh->add_sum = src_sum;
	if (neon_sum == src_sum)
		return 0;
	else
		return -1;
	return 0;
}
#else
/* check: 0-success  -1:fail */
int verify_addsum(void *mem_base, u32 size)
{
	u32 *buf;
	u32 count;
	u32 src_sum;
	u32 sum;
	sbrom_toc1_head_info_t *bfh;

	bfh = (sbrom_toc1_head_info_t *)mem_base;
	/*generate checksum*/
	src_sum = bfh->add_sum;
	bfh->add_sum = STAMP_VALUE;
	count = size >> 2;
	sum = 0;
	buf = (u32 *)mem_base;
	do
	{
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
		sum += *buf++;
	}while( ( count -= 4 ) > (4-1) );

	while( count-- > 0 )
		sum += *buf++;

	bfh->add_sum = src_sum;

//	printf("sum=%x\n", sum);
//	printf("src_sum=%x\n", src_sum);
	if( sum == src_sum )
		return 0;
	else
		return -1;
}
#endif

u32 g_mod( u32 dividend, u32 divisor, u32 *quot_p)
{
	if (divisor == 0) {
		*quot_p = 0;
		return 0;
	}
	if (divisor == 1) {
		*quot_p = dividend;
		return 0;
	}

	for (*quot_p = 0; dividend >= divisor; ++(*quot_p))
		dividend -= divisor;
	return dividend;
}

u8  *get_page_buf( void )
{

	return (u8 *)(CONFIG_SYS_DRAM_BASE + 1024 * 1024);
}

char get_uart_input(void)
{

	char c = 0;
	if (sunxi_serial_tstc()) {
		c = sunxi_serial_getc();
		printf("key press : %c\n", c);
		/* test time: 30 ms */
		/*
		if (get_sys_ticks() - start > 30)
			break;
		*/
	}
	return c;
}

static uint8_t uboot_func_mask;
void set_uboot_func_mask(uint8_t mask)
{
	uboot_func_mask |= mask;
}

uint8_t get_uboot_func_mask(uint8_t mask)
{
	return uboot_func_mask & mask;
}

__weak int sunxi_board_init(void)
{
	return 0;
}

__weak uint8_t sunxi_board_late_init(void)
{
	return 0;
}
