/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 AllwinnerTech.
 *
 * Authors:
 *   liush <liush@allwinnertech.com>
 */

#ifndef __SERIAL_SUNXI_UART_H__
#define __SERIAL_SUNXI_UART_H__

#include <sbi/sbi_types.h>

void sunxi_uart_putc(char ch);

int sunxi_uart_getc(void);

int sunxi_uart_init(unsigned long base);

#endif
