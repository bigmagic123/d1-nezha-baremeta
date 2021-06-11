// SPDX-License-Identifier: BSD-2-Clause
#ifndef __SUNXI_UART_H
#define __SUNXI_UART_H

void sunxi_uart_putc(char ch);
int sunxi_uart_getc(void);
int sunxi_uart_init(unsigned long base);

static inline void pr_info(char *p)
{
	while (*p != 0)
		sunxi_uart_putc(*p++);
}

#endif
