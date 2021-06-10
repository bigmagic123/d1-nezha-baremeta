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
	 unsigned long long cnt = 0;

	 asm volatile("csrr %0, time\n"
		: "=r"(cnt)
		:
		: "memory"
		);

	return cnt;
}

/*
 * get current time.(millisecond)
 */
u32 get_sys_ticks(void)
{
	return (u32)get_arch_counter() / 24000;
}

/*
 * get current time.(microsecond)
 */
u32 timer_get_us(void)
{
	return (u32)get_arch_counter() / 24;
}

__weak void udelay(unsigned long us)
{
	u64 t1, t2;

	t1 = get_arch_counter();
	t2 = t1 + us * 24;
	do {
		t1 = get_arch_counter();
	} while (t2 >= t1);
}

__weak void mdelay(unsigned long ms)
{
	udelay(ms * 1000);
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
}

__weak void timer_exit(void)
{
}


