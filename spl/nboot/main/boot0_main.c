/*
 * (C) Copyright 2018
* SPDX-License-Identifier:	GPL-2.0+
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/rtc.h>
#include <arch/gpio.h>
#ifdef CFG_DDR_SOFT_TRAIN
#include <arch/efuse.h>
#endif

static void update_uboot_info(phys_addr_t uboot_base, phys_addr_t optee_base,
		phys_addr_t monitor_base, phys_addr_t rtos_base, u32 dram_size,
		u16 pmu_type, u16 uart_input, u16 key_input);
static int boot0_clear_env(void);

void main(void)
{
	int dram_size;
	int status;
	phys_addr_t  uboot_base = 0, optee_base = 0, monitor_base = 0, \
				rtos_base = 0, opensbi_base = 0;
	u16 pmu_type = 0, key_input = 0; /* TODO: set real value */

	sunxi_serial_init(BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6);
	printf("HELLO! BOOT0 is starting!\n");
	printf("BOOT0 commit : %s\n", BT0_head.hash);
	sunxi_set_printf_debug_mode(BT0_head.prvt_head.debug_mode);

	status = sunxi_board_init();
	if(status)
		goto _BOOT_ERROR;

	if (rtc_probe_fel_flag()) {
		rtc_clear_fel_flag();
		goto _BOOT_ERROR;
#ifdef CFG_SUNXI_PHY_KEY
	} else if (check_update_key(&key_input)) {
		goto _BOOT_ERROR;
#endif
	} else if (BT0_head.prvt_head.enable_jtag) {
		printf("enable_jtag\n");
		boot_set_gpio((normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio, 5, 1);
	}

#ifdef FPGA_PLATFORM
	dram_size = mctl_init((void *)BT0_head.prvt_head.dram_para);
#else
#ifdef CFG_DDR_SOFT_TRAIN
	if (BT0_head.prvt_head.dram_para[30] & (1 << 11))
		neon_enable();
#endif
	dram_size = init_DRAM(0, (void *)BT0_head.prvt_head.dram_para);
#endif
	if(!dram_size)
		goto _BOOT_ERROR;
	else {
		printf("dram size =%d\n", dram_size);
	}

	char uart_input_value = get_uart_input();

	if (uart_input_value == '2') {
		sunxi_set_printf_debug_mode(3);
		printf("detected user input 2\n");
		goto _BOOT_ERROR;
	} else if (uart_input_value == 'd') {
		sunxi_set_printf_debug_mode(8);
		printf("detected user input d\n");
	}

	mmu_enable(dram_size);
	malloc_init(CONFIG_HEAP_BASE, CONFIG_HEAP_SIZE);
	status = sunxi_board_late_init();
	if (status)
		goto _BOOT_ERROR;

	status = load_package();
	if(status == 0 )
		load_image(&uboot_base, &optee_base, &monitor_base, &rtos_base, &opensbi_base);
	else
		goto _BOOT_ERROR;

	update_uboot_info(uboot_base, optee_base, monitor_base, rtos_base, dram_size,
			pmu_type, uart_input_value, key_input);
	mmu_disable( );

	printf("Jump to second Boot.\n");
	if (opensbi_base) {
			boot0_jmp_opensbi(opensbi_base, uboot_base);
	} else if (monitor_base) {
		struct spare_monitor_head *monitor_head =
			(struct spare_monitor_head *)((phys_addr_t)monitor_base);
		monitor_head->secureos_base = optee_base;
		monitor_head->nboot_base = uboot_base;
		boot0_jmp_monitor(monitor_base);
	} else if (optee_base)
		boot0_jmp_optee(optee_base, uboot_base);
	else if (rtos_base) {
		printf("jump to rtos\n");
		boot0_jmp(rtos_base);
	}
	else
		boot0_jmp(uboot_base);

	while(1);
_BOOT_ERROR:
	boot0_clear_env();
	boot0_jmp(FEL_BASE);

}

static void update_uboot_info(phys_addr_t uboot_base, phys_addr_t optee_base,
		phys_addr_t monitor_base, phys_addr_t rtos_base, u32 dram_size,
		u16 pmu_type, u16 uart_input, u16 key_input)
{
	if (rtos_base)
		return;

	uboot_head_t  *header = (uboot_head_t *) uboot_base;
	struct sbrom_toc1_head_info *toc1_head = (struct sbrom_toc1_head_info *)CONFIG_BOOTPKG_BASE;

	header->boot_data.boot_package_size = toc1_head->valid_len;
	header->boot_data.dram_scan_size = dram_size;
	memcpy((void *)header->boot_data.dram_para, &BT0_head.prvt_head.dram_para, 32 * sizeof(int));

	if(monitor_base)
		header->boot_data.monitor_exist = 1;
	if(optee_base)
		header->boot_data.secureos_exist = 1;
#ifndef CONFIG_RISCV
	header->boot_data.func_mask |= get_uboot_func_mask(UBOOT_FUNC_MASK_ALL);
#endif
	update_flash_para(uboot_base);

	header->boot_data.pmu_type = pmu_type;
	header->boot_data.uart_input = uart_input;
	header->boot_data.key_input = key_input;
	header->boot_data.debug_mode = sunxi_get_printf_debug_mode();
}

static int boot0_clear_env(void)
{
	sunxi_board_exit();
	sunxi_board_clock_reset();
	mmu_disable();
	mdelay(10);

	return 0;
}
