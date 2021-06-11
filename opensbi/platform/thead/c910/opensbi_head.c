/*
 * SPDX-License-Identifier: BSD-2-Clause
 * Header for semelis
 *
 */

#include "private_opensbi.h"

/*#define BROM_FILE_HEAD_SIZE		((sizeof(struct private_opensbi_head)) & 0x00FFFFF)*/
#define BROM_FILE_HEAD_SIZE		(0x400 & 0x00FFFFF)
#define BROM_FILE_HEAD_BIT_10_1		((BROM_FILE_HEAD_SIZE & 0x7FE) >> 1)
#define BROM_FILE_HEAD_BIT_11		((BROM_FILE_HEAD_SIZE & 0x800) >> 11)
#define BROM_FILE_HEAD_BIT_19_12	((BROM_FILE_HEAD_SIZE & 0xFF000) >> 12)
#define BROM_FILE_HEAD_BIT_20		((BROM_FILE_HEAD_SIZE & 0x100000) >> 20)

#define BROM_FILE_HEAD_SIZE_OFFSET	((BROM_FILE_HEAD_BIT_20 << 31) | \
									(BROM_FILE_HEAD_BIT_10_1 << 21) | \
									(BROM_FILE_HEAD_BIT_11 << 20) | \
									(BROM_FILE_HEAD_BIT_19_12 << 12))
#define JUMP_INSTRUCTION		(BROM_FILE_HEAD_SIZE_OFFSET | 0x6f)

struct private_opensbi_head  opensbi_head __attribute__ ((section(".head_data"))) =
{
		JUMP_INSTRUCTION,
		"opensbi",
		0,
		0,
		0,
		0,
		{0},
		{0},
		FW_TEXT_START
};
