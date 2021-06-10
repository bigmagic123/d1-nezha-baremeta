/*
 * (C) Copyright 2020
* SPDX-License-Identifier:	GPL-2.0+
 * wangwei <ouyangkun@allwinnertech.com>
 */

#include <common.h>
#include <private_boot0.h>
#include <arch/uart.h>
#include <arch/efuse.h>
#include <arch/watchdog.h>

void main(void)
{
	sunxi_serial_init(BT0_head.prvt_head.uart_port,
			  (void *)BT0_head.prvt_head.uart_ctrl, 6);
	printf("HELLO! BOOT0 for offline secure burn is starting!\n");
	printf("BOOT0 commit : %s\n", BT0_head.hash);

	printf("ready to burn secure bit\n");
	sid_set_security_mode();
	printf("burn done, start watchdog to restart\n");
	wdt_start(1);
	while (1)
		;
}
