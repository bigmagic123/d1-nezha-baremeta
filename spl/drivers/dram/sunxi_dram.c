/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
* SPDX-License-Identifier:	GPL-2.0+
* SPDX-License-Identifier:	GPL-2.0+
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <weidonghui@allwinnertech.com>
 *
 * Some init for sunxi platform.
 *
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <private_boot0.h>
#include <private_toc.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <asm/io.h>
#include <arch/gpio.h>

int sunxi_read_gpadc_vol(int channel);


u32 get_boot_dram_update_flag(uint32_t *dram_para)
{
	return (dram_para[23] >> 31) & 0x1;
}


#ifdef CFG_SUNXI_SELECT_DRAM_PARA
int sunxi_dram_handle(void)
{
	int i, vaild_para = 1, io_en = 0, dram_para_total = 7;
	uint32_t *dram_para, select_dram_para = 0;
	boot_extend_head_t *select_para;
	u32 vol_range[] = {163, 382, 608, 811, 1050, 1315, 1569, 1800}; /*mV*/
#ifdef CFG_SUNXI_SBOOT
	dram_para   = (uint32_t *)toc0_config->dram_para;
	select_para = (boot_extend_head_t *)&sboot_head.extd_head;
#elif CFG_SUNXI_FES
	dram_para   = (uint32_t *)fes1_head.prvt_head.dram_para;
	select_para = (boot_extend_head_t *)&fes1_head.fes_union_addr.extd_head;
#else
	dram_para   = (uint32_t *)BT0_head.prvt_head.dram_para;
	select_para = (boot_extend_head_t *)&BT0_head.fes_union_addr.extd_head;
#endif

	if (!get_boot_dram_update_flag(dram_para)) {
		if (!strncmp((char *)select_para->magic, DRAM_EXT_MAGIC, strlen(DRAM_EXT_MAGIC))) {
#ifdef AUTO_DRAM_DEBUG
			for (i = 0; i < 32; i++) {
				printf("dram_para[%d]:0x%x\n", i, dram_para[i]);
			}
			printf("select_mode:%d\n", select_para->select_mode);
			printf("adc_channel:%d\n", select_para->gpadc_channel);
#endif

			boot_set_gpio((void *)select_para->dram_select_gpio, 6, 1);
			if (select_para->select_mode == 1) {
				for (i = 0; i < 4; i++) {
					if (select_para->dram_select_gpio[i].port == 0)
						continue;
					select_dram_para |= (PIO_ONE_PIN_DATA((select_para->dram_select_gpio[i].port), (select_para->dram_select_gpio[i].port_num)) << i);
				}
				dram_para_total = 15;
			} else if (select_para->select_mode != 0) {
#ifdef CFG_GPADC_KEY
				int adc_vol = sunxi_read_gpadc_vol(select_para->gpadc_channel);
				for (select_dram_para = 0; select_dram_para < (sizeof(vol_range)/sizeof(vol_range[0]) - 1); select_dram_para++) {
					if ((adc_vol < (vol_range[select_dram_para+1] + vol_range[select_dram_para]) / 2)) {
						break;
					}
				}
#endif
				if (select_para->select_mode == 3) {
					for (i = 0; i < 4; i++) {
						if (select_para->dram_select_gpio[i].port == 0)
							continue;
						io_en = PIO_ONE_PIN_DATA((select_para->dram_select_gpio[i].port), (select_para->dram_select_gpio[i].port_num));
						break;
					}
					dram_para_total = 15;
				}
			}
			select_dram_para += (io_en * 8);
			if (select_dram_para != 0) {
				if (select_para->dram_para[select_dram_para-1][0] == 0) {
					printf("dram para%d invalid use default para\n", select_dram_para);
					select_dram_para = 0;
				} else {
					memcpy(dram_para, select_para->dram_para[select_dram_para-1], sizeof(select_para->dram_para[0]));
				}
			}

			for (i = 0; i < dram_para_total; i++) {
				if (select_para->dram_para[i][0] == 0) {
					continue;
				}
				vaild_para++;
			}
			printf("vaild para:%d  select dram para%d\n", vaild_para,  select_dram_para);
#ifdef AUTO_DRAM_DEBUG
			while (1) {
				char uart_val = get_uart_input();
				if (uart_val == 'q')
					break;
				if (uart_val == 'w') {
					for (i = 0; i < 4; i++) {
						if (!select_para->dram_select_gpio[i].port)
							continue;
						printf("GPIO%c%d:0x%x\n", 'A'-1 + select_para->dram_select_gpio[i].port,
							select_para->dram_select_gpio[i].port_num,
							PIO_ONE_PIN_DATA((select_para->dram_select_gpio[i].port), (select_para->dram_select_gpio[i].port_num)));
					}
					printf("gpadc val:0x%x\n", sunxi_read_gpadc_vol(select_para->gpadc_channel));
				}
				mdelay(10);
			}
			for (i = 0; i < 32; i++) {
				printf("dram_para[%d]:0x%x\n", i, dram_para[i]);
			}
#endif
		} else {
			printf("extd_head bad magic\n");
			return -1;
		}
	} else {
		printf("dram return write ok\n");
	}

	if (sunxi_get_printf_debug_mode() >= 4) {
		for (i = 0; i < 32; i++) {
			printf("dram_para[%d]:0x%x\n", i, dram_para[i]);
		}
	}
	return 0;
}
#endif

