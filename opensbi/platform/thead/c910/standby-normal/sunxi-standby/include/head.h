// SPDX-License-Identifier: BSD-2-Clause
#ifndef __HEAD_H
#define __HEAD_H

#include "stdint.h"

#define PMU_NUM 4

typedef struct __DRAM_PARA {
	/*normal configuration */
	uint32_t dram_clk;
	/*dram_type DDR2: 2 DDR3: 3 LPDDR2: 6 LPDDR3: 7 DDR3L: 31 */
	uint32_t dram_type;
	/*uint32_t lpddr2_type;  //LPDDR2 type  S4:0  S2:1  NVM:2 */
	uint32_t dram_zq;	/*do not need */
	uint32_t dram_odt_en;

	/*control configuration */
	uint32_t dram_para1;
	uint32_t dram_para2;

	/*timing configuration */
	uint32_t dram_mr0;
	uint32_t dram_mr1;
	uint32_t dram_mr2;
	uint32_t dram_mr3;
	uint32_t dram_tpr0;	/*DRAMTMG0 */
	uint32_t dram_tpr1;	/*DRAMTMG1 */
	uint32_t dram_tpr2;	/*DRAMTMG2 */
	uint32_t dram_tpr3;	/*DRAMTMG3 */
	uint32_t dram_tpr4;	/*DRAMTMG4 */
	uint32_t dram_tpr5;	/*DRAMTMG5 */
	uint32_t dram_tpr6;	/*DRAMTMG8 */

	/*reserved for future use */
	uint32_t dram_tpr7;
	uint32_t dram_tpr8;
	uint32_t dram_tpr9;
	uint32_t dram_tpr10;
	uint32_t dram_tpr11;
	uint32_t dram_tpr12;
	uint32_t dram_tpr13;
	uint32_t reserved[8];
} __dram_para_t;

typedef struct __pin_para {
	unsigned char pin_grp;
	unsigned char pin_num;
	unsigned char mode;
	unsigned char pupd;
	unsigned char drv;
	unsigned char reserved[3];
} __pin_para_t;

typedef struct __twi_para {
	uint32_t reg_base;
	uint32_t reserved[3];
	__pin_para_t sda;
	__pin_para_t sck;
} __twi_para_t;

typedef struct sram_head {
	uint32_t entry;
	uint32_t crc_start;
	uint32_t crc_len;
	uint32_t crc_before;
	uint32_t crc_after;
	uint32_t time_to_wakeup_ms;
	uint32_t crc_enable;
	uint32_t standby_type;
	uint32_t vdd_cpua;
	uint32_t vdd_cpub;
	uint32_t vdd_sys;
	uint32_t vcc_pll;
	uint32_t vcc_io;
	uint32_t vdd_res1;
	__dram_para_t dram_para; /* 32 words*/
	uint32_t vdd_res2;
	uint32_t pmu_count;
	uint32_t pmu_port;
	uint32_t pmu_para;
	unsigned char pmu_id[PMU_NUM];
	unsigned char pmu_addr[PMU_NUM];
	__twi_para_t twi_para;
} sram_head_t;  /* total 52 words */

#endif /* __HEAD_H */
