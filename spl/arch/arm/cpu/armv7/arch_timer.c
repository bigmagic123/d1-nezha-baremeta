/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <asm/io.h>

/*
 * 64bit arch timer.CNTPCT
 * Freq = 24000000Hz
 */
static inline u64 get_arch_counter(void)
{
	u32 low=0, high = 0;
	asm volatile("mrrc p15, 0, %0, %1, c14"
		: "=r" (low), "=r" (high)
		:
		: "memory");
	return (((u64)high)<<32 | low);
}

/*
 * get current time.(millisecond)
 */
u32 get_sys_ticks(void)
{
	return (u32)get_arch_counter()/24000;
}

/*
 * get current time.(microsecond)
 */
u32 timer_get_us(void)
{
	return (u32)get_arch_counter()/24;
}

__weak void udelay(unsigned long us)
{
	u64 t1, t2;

	t1 = get_arch_counter();
	t2 = t1 + us*24;
	do
	{
		t1 = get_arch_counter();
	}
	while(t2 >= t1);
}

__weak void mdelay(unsigned long ms)
{
	udelay(ms*1000);
}

__weak void __usdelay(unsigned long us)
{
	udelay(us);
}

__weak void __msdelay(unsigned long ms)
{
	mdelay(ms);
}

__weak int timer_init(void)
{
	return 0;
}

/************************************************************
 * sdelay() - simple spin loop.  Will be constant time as
 *  its generally used in bypass conditions only.  This
 *  is necessary until timers are accessible.
 *
 *  not inline to increase chances its in cache when called
 *************************************************************/
__weak void sdelay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

__weak void timer_exit(void)
{

}


