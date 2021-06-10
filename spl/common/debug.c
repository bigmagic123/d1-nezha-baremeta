/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
#include <common.h>

void u8_to_string_hex( u8 input, char * str )
{
	int i;
	static char base[] = "0123456789abcdef";

	for( i = 1; i >= 0; --i )
	{
		str[i] = base[input&0xf];
		input >>= 4;
	}

	str[2] = '\0';

	return;
}

int snprintf(char *buf, size_t size, const char *fmt, ...);
void ndump(u8 *buf, int count)
{
	char line[50];
	char *tmp = line;
	char *in  = (char *)buf;
	int i;
	printf("inbuf:%x\n", (phys_addr_t)buf);
	for (i = 0; i < count; i++) {
		snprintf(tmp, 5, "%02x ", *((uint8_t *)in));
		tmp += 3;
		if (((i % 16) == 15)) {
			printf("%s\n", line);
			tmp = line;
		}
		in++;
	}
	if (((i % 16) != 0)) {
		printf("%s\n", line);
		tmp = line;
	}
}

void __assert_fail(const char *assertion, const char *file, unsigned line,
		   const char *function)
{
	/* This will not return */
	printf("%s:%u: %s: Assertion `%s' failed.", file, line, function,
	      assertion);
	while(1);
}

#if CFG_SUNXI_SIMULATE_BOOT0 /* help build boot0 for simulation */
#include <asm/io.h>
#define SYS_CFG_BASE (SUNXI_SYSCRL_BASE)
#define SYS_MSG (SYS_CFG_BASE + 0xB0)
#define DBUG_CTRL (SYS_CFG_BASE + 0xC0)
#define DBUG_DATA (SYS_CFG_BASE + 0xC4)

void write_debug_reg(u32 data)
{
	u32 temp = 0;
	writel(data, DBUG_DATA);
	temp = (1 << 31) | (2 << 28) | (SYS_MSG & 0x0fffffff);
	writel(temp, DBUG_CTRL);
}

void pattern_goto(u32 step)
{
	u32 rdata;
	rdata = (1 << 26) | (step & 0xFFFF);
	write_debug_reg(rdata);
	write_debug_reg(0);
}

void pattern_mod_goto(u32 mod, u32 step)
{
}

void pattern_end(u32 pass)
{
	if (pass == 1)
		pattern_goto(0xfffc);
	else
		pattern_goto(0xfffD);
}
#endif
