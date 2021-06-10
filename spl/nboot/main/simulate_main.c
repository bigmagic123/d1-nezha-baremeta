// * SPDX-License-Identifier:	GPL-2.0+
#include "common.h"
#include <private_boot0.h>
#include <arch/rtc.h>

struct {
	uint8_t padding[(CFG_SUNXI_SIM_SIZE_KB - 4) * 1024];
	uint8_t tail[16];
} padding = { { 0 }, "pad tail" };

void main(void)
{
	if (rtc_probe_fel_flag()) {
		goto _BOOT_ERROR;
	}
	pattern_end(1);
	while (BT0_head.boot_head.jump_instruction & padding.tail[0])
		;
_BOOT_ERROR:
	boot0_jmp(FEL_BASE);
}
