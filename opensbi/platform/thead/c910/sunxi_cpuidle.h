/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 ALLWINNER Corporation.
 *
 * Authors:
 *   liush <liush@allwinnertech.com>
 */

#ifndef __SBI_IDLE_H__
#define __SBI_IDLE_H__

/**  */
#define SBI_GPR_zero			0
#define SBI_GPR_ra			1
#define SBI_GPR_sp			2
#define SBI_GPR_gp			3
#define SBI_GPR_tp			4
#define SBI_GPR_t0			5
#define SBI_GPR_t1			6
#define SBI_GPR_t2			7
#define SBI_GPR_s0			8
#define SBI_GPR_s1			9
#define SBI_GPR_a0			10
#define SBI_GPR_a1			11
#define SBI_GPR_a2			12
#define SBI_GPR_a3			13
#define SBI_GPR_a4			14
#define SBI_GPR_a5			15
#define SBI_GPR_a6			16
#define SBI_GPR_a7			17
#define SBI_GPR_s2			18
#define SBI_GPR_s3			19
#define SBI_GPR_s4			20
#define SBI_GPR_s5			21
#define SBI_GPR_s6			22
#define SBI_GPR_s7			23
#define SBI_GPR_s8			24
#define SBI_GPR_s9			25
#define SBI_GPR_s10			26
#define SBI_GPR_s11			27
#define SBI_GPR_t3			28
#define SBI_GPR_t4			29
#define SBI_GPR_t5			30
#define SBI_GPR_t6			31
#define SBI_GPR_MAX			32
#define SBI_REGS_OFFSET(x) ((SBI_GPR_##x) * 8)

#define SBI_PMP_addr0                    0
#define SBI_PMP_addr1                    1
#define SBI_PMP_addr2                    2
#define SBI_PMP_addr3                    3
#define SBI_PMP_addr4                    4
#define SBI_PMP_addr5                    5
#define SBI_PMP_addr6                    6
#define SBI_PMP_addr7                    7
#define SBI_PMP_cfg0                     8
#define SBI_PMP_MAX                      9
#define PMP(X)  SBI_PMP_##X

#define SBI_CSR_minstret                   0
#define SBI_CSR_mstatus                    1
#define SBI_CSR_medeleg                    2
#define SBI_CSR_mideleg                    3
#define SBI_CSR_mtvec                      4
#define SBI_CSR_mie                        5
#define SBI_CSR_mcounteren                 6   
#define SBI_CSR_mscratch                   7
#define SBI_CSR_mepc                       8
#define SBI_CSR_mcause                     9
#define SBI_CSR_mcounterwen                10
#define SBI_CSR_mcounterinten              11
#define SBI_CSR_mcycle                     12
#define SBI_CSR_MMAX                   13
#define CSR_M(X)  SBI_CSR_##X

#define SBI_CSR_satp                       0
#define SBI_CSR_sstatus                    1 
#define SBI_CSR_sie                        2
#define SBI_CSR_stvec                      3
#define SBI_CSR_scounteren                 4
#define SBI_CSR_sscratch                   5
#define SBI_CSR_sepc                       6
#define SBI_CSR_scause                     7
#define SBI_CSR_sxstatus                   8
#define SBI_CSR_shcr                       9
#define SBI_CSR_scounterinten              10
#define SBI_CSR_scycle                     11
#define SBI_CSR_smir                       12
#define SBI_CSR_smel                       13
#define SBI_CSR_smeh                       14
#define SBI_CSR_smcir                      15
#define SBI_CSR_SMAX                      16
#define CSR_S(X)  SBI_CSR_##X

#define C906_EXTEN_mcor                 0
#define C906_EXTEN_mhcr                 1
#define C906_EXTEN_mhint                2
#define C906_EXTEN_mxstatus             3
#define C906_EXTEN_MAX                  4
#define C906_EXTEN(X)  C906_EXTEN_##X

#endif
